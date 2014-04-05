#include "winglbase.h"
#include "glextensionfuncs.h"
#include "matrixstack.h"
#include "shader.h"
#include "mesh.h"
#include "debug.h"
#include "texture.h"
#include "obj.h"
#include "camera.h"
#include "framebuffer.h"
#include "utils.h"
#include "shapes.h"
#include "sampler.h"
#include "constants.h"

#define MOUSE_SENSITIVITY 20.f
#define MOVESPEED 0.05f
#define FOV_Y 45

MatrixStack mats;
Matrix camMv;
Matrix camPj;
FirstPersonCamera cam;
Framebuffer *fbuf; // final compositing framebuffer
Framebuffer *gbufs; // g-buffers for deferred
WinGLBase *window;

struct Light {
    fl3 pos;
    fl3 power; // light power in each channel
    float size;

};

std::vector<Light> lights;

long OnResize(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam)
{
    wnd.resize(LOWORD(lparam), HIWORD(lparam));
    if (fbuf) {
        fbuf->resize(wnd.getWidth(), wnd.getHeight());
    }
    if (gbufs) {
        gbufs->resize(wnd.getWidth(), wnd.getHeight());
    }
    cam.init(FOV_Y, wnd.getAspect(), 1.f, 500.f);
    glViewport(0, 0, wnd.getWidth(), wnd.getHeight());
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    fbuf = 0;
    gbufs = 0;
    window = new WinGLBase(hInstance, 1024, 768);
    window->addMessageHandler(WM_SIZE, OnResize);
    window->showWindow(nShowCmd);
    glDebugMessageCallbackARB(errorCallback, NULL);
	// NSIGHT doesn't support this
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    cam.setPos(fl3(0, 10, 10));

    ShaderProgram dprepass ("tiledprepass.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
    ShaderProgram tileCompute ("tiledcompute.glsl", Shader::COMPUTE_SHADER);
    ShaderProgram lightRender ("light.glsl", Shader::VERTEX_SHADER | Shader::TESSELLATION_SHADER | Shader::GEOMETRY_SHADER | Shader::FRAGMENT_SHADER);

    mats.initUniformLocs(0,1);

    FramebufferParams params;
    params.width = window->getWidth();
    params.height = window->getHeight();
    params.numSamples = 1;
    params.numMrts = 1;
    params.colorEnable = true;
    params.depthEnable = true;
    params.format = GL_RGBA32F;
    params.depthFormat = GL_DEPTH_COMPONENT32F;
    params.type = GL_TEXTURE_2D;
    params.filter = GL_LINEAR;
    fbuf = new Framebuffer(params);

    // g buffers we need
    // ambient color
    // diffuse albedo
    // specular color
    // normals
    // NOT IDEAL: adding another one for depth because you can't bind depth textures to image units
    params.numMrts = 5;
    // compute shader causing gl state mismatch problems when using sampler2DMS
    //params.type = GL_TEXTURE_2D_MULTISAMPLE;
    gbufs = new Framebuffer(params);

    glEnable(GL_SAMPLE_SHADING);
    glMinSampleShading(1.0); // force full supersampling

    Sampler smapRender;
    glSamplerParameteri(smapRender.getID(), GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glSamplerParameteri(smapRender.getID(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(smapRender.getID(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(smapRender.getID(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(smapRender.getID(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    std::vector<PTvert> verts;
    PTvert v;
    v.pos_ = fl3(0,0,0); v.tex_ = fl3(0,0,0); verts.push_back(v);
    v.pos_ = fl3(1,0,0); v.tex_ = fl3(1,0,0); verts.push_back(v);
    v.pos_ = fl3(1,1,0); v.tex_ = fl3(1,1,0); verts.push_back(v);
    v.pos_ = fl3(0,1,0); v.tex_ = fl3(0,1,0); verts.push_back(v);
    std::vector<GLubyte> inds;
    inds.push_back(0x0); inds.push_back(0x1); inds.push_back(0x2); inds.push_back(0x3);
    InterleavedMesh<PTvert, GLubyte> quad (GL_QUADS);
    quad.addAttrib(0, AttributeInfoSpec<GLfloat>(3)).addAttrib(1, AttributeInfoSpec<GLfloat>(3));
    quad.addVerts(verts).addInds(inds).finalize();

    std::vector<PTNvert> gverts;
    PTNvert gv;
    gv.norm_ = fl3(0, 1.0f, 0);
    gv.pos_ = fl3(-50, 0, -50); gv.tex_ = fl3(0, 0, 0); gverts.push_back(gv);
    gv.pos_ = fl3(-50, 0, 50); gv.tex_ = fl3(1, 0, 0); gverts.push_back(gv);
    gv.pos_ = fl3(50, 0, 50); gv.tex_ = fl3(1, 1, 0); gverts.push_back(gv);
    gv.pos_ = fl3(50, 0, -50); gv.tex_ = fl3(0, 1, 0); gverts.push_back(gv);
    InterleavedMesh<PTNvert, GLubyte> gquad (GL_QUADS);
    gquad.addAttrib(0, AttributeInfoSpec<GLfloat>(3)).addAttrib(1, AttributeInfoSpec<GLfloat>(3)).addAttrib(2, AttributeInfoSpec<GLfloat>(3));
    gquad.addVerts(gverts).addInds(inds).finalize();

    Obj testobj ("../../assets/ServerBot1.obj");
    Mesh<GLubyte> *lightMesh = createIcosahedron(true);
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    int closestLight = -1;
    float closestDepth = -FLT_MAX;
    fl3 intersectPoint;
    fl3 originalCenter;

    // populate some lights
    GLuint lightsTex;
    GLuint lightsBuf;
    {
        Light l;
        l.pos = fl3(10, 10, 10);
        l.power = fl3(100.0, 100.0, 100.0);
        l.size = 200.0;
        lights.push_back(l);
        l.power = fl3(100.0, 0, 0);
        l.pos = fl3(-10, 10, 10);
        lights.push_back(l);
        l.power = fl3(0, 100.0, 0);
        l.pos = fl3(0, 20, 10);
        lights.push_back(l);
        l.power = fl3(0, 0, 100.0);
        l.pos = fl3(0, 0, 10);
        lights.push_back(l);


        // for the shaders, we want to store the light properties in a texture buffer
        glGenBuffers(1, &lightsBuf);
        glBindBuffer(GL_TEXTURE_BUFFER, lightsBuf);
        glBufferData(GL_TEXTURE_BUFFER, sizeof(Light)*lights.size(), &lights[0], GL_STATIC_READ);
        glGenTextures(1, &lightsTex);
        glBindTexture(GL_TEXTURE_BUFFER, lightsTex);
        glTexBufferRange(GL_TEXTURE_BUFFER, GL_R32F, lightsBuf, 0, sizeof(Light)*lights.size());
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
        glBindTexture(GL_TEXTURE_BUFFER, 0);
    }

    // create texture buffer for light index lists and texture image for light grid
    GLuint lightIndexListsBuffer;
    GLuint lightIndexListsTex;
    GLuint lightGridTex;
    const int2 tileSize (32, 32); // constant size for tiles in pixels
    // how many tiles are there in the image?
    const unsigned int numTilesX = (gbufs->getWidth() + tileSize.x - 1) / tileSize.x;
    const unsigned int numTilesY = (gbufs->getHeight() + tileSize.y - 1) / tileSize.y;
    {
        glGenBuffers(1, &lightIndexListsBuffer);
        glBindBuffer(GL_TEXTURE_BUFFER, lightIndexListsBuffer);
        // max size of light index lists is number of tiles * number of lights total
        // light indices must be able to hold the total number of lights, so we will use 8 bits for now (up to 256 lights)
        const unsigned int bufferSize = 1*lights.size()*numTilesX*numTilesY;
        glBufferData(GL_TEXTURE_BUFFER, bufferSize, 0, GL_DYNAMIC_READ);
        glGenTextures(1, &lightIndexListsTex);
        glBindTexture(GL_TEXTURE_BUFFER, lightIndexListsTex);
        glTexBufferRange(GL_TEXTURE_BUFFER, GL_R8UI, lightIndexListsBuffer, 0, bufferSize);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
        glBindTexture(GL_TEXTURE_BUFFER, 0);

        // now make the light grid texture
        glGenTextures(1, &lightGridTex);
        glBindTexture(GL_TEXTURE_2D, lightGridTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8UI, numTilesX, numTilesY, 0, GL_RG_INTEGER, GL_UNSIGNED_BYTE, 0);
    }

    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0,0,0,0);

    while (window->isActive()) {
        window->update();

        if (window->isKeyPress(KEY_ESC)) {
            window->close();
            break;
        }
        if (window->isMousePress(MOUSE_RIGHT)) {
            window->hideCursor();
            window->holdCursor(true);
        } else if (window->isMouseRelease(MOUSE_RIGHT)) {
            window->showCursor();
            window->holdCursor(false);
        }
        if (window->isMouseDown(MOUSE_RIGHT)) {
            fl2 dxdy = window->getMouseNorm_dxdy();
            cam.rotate(fl2(-MOUSE_SENSITIVITY * dxdy.y, -MOUSE_SENSITIVITY * dxdy.x));
        }
        cam.toMatrixMv(camMv);
        cam.toMatrixPj(camPj);
        if (window->isMouseDown(MOUSE_LEFT)) {
            // generate a ray in normalized device coordinates
            fl2 mpos = window->getMouseNormPos();
            // transform to ndc (-1, 1)
            mpos *= 2;
            mpos -= fl2(1,1);
            mpos.y *= -1;

            // see http://antongerdelan.net/opengl/raycasting.html
            // transform to homogeneous clip coordinates
            fl3 ray_clip;
            ray_clip.x = mpos.x; ray_clip.y = mpos.y; ray_clip.z = -1.0f;
            // transform to camera/eye space
            Matrix invproj;
            camPj.getInverse(invproj);
            fl3 ray = invproj.multiplyPoint(ray_clip);
            ray.normalize();

            if (window->isMousePress(MOUSE_LEFT)) {
                float closest = FLT_MAX; // distance along ray to closest light, start at max positive value

                // see if we select a light
                for (unsigned int i = 0; i < lights.size(); i++) {
                    Light &li = lights.at(i);

                    // do a ray-sphere intersection for the light
                    // transform light center to view space

                    fl3 wpos = camMv.multiplyPoint(li.pos);

                    // ray-sphere intersection formula
                    const fl3 oc = -wpos;
                    const float loc = fl3::dot(ray, oc);
                    const float test = (loc*loc - fl3::dot(oc, oc) + 1);// + li.size*li.size); // making interaction use radius 1 rather than light size since it can be huge

                    if (test >= 0) {
                        // we have a ray sphere intersection
                        // now find its exact location
                        const float d0 = -loc - sqrt(test);
                        const float d1 = -loc + sqrt(test);
                        if (d0 > 0 && d0 < closest) {
                            // d0 must be the closest
                            closest = d0;
                        } else if (d1 > 0 && d1 < closest) {
                            closest = d1;
                        }
                        if (d0 > 0 || d1 > 0) {
                            closestLight = i;
                            closestDepth = closest;
                            intersectPoint = ray * closestDepth; // intersect point in view space
                            originalCenter = li.pos;
                        }
                        // we have an intersection and it's the closest light found so far
                    }
                }
                // now we should have the closest light to the camera which was moused over
            }
            if (closestLight >= 0) {
                // we have a light selected and are possibly dragging it
                fl3 translation = (ray * closestDepth) - intersectPoint; // translation in eye space
                // inverse camera matrix should put it back in world space
                Matrix inverseCam;
                camMv.getInverse(inverseCam);
                fl3 finaltrans = inverseCam.multiplyVector(translation);
                // now move the light by this amount
                Light &li = lights.at(closestLight);
                li.pos = originalCenter + finaltrans;
                // need to update the corresponding light data in the GPU buffer
                glBindBuffer(GL_TEXTURE_BUFFER, lightsBuf);
                glBufferSubData(GL_TEXTURE_BUFFER, sizeof(Light)*closestLight, sizeof(fl3), &li.pos);
            }
        }
        if (window->isMouseRelease(MOUSE_LEFT)) {
            // deselect the closest light
            closestLight = -1;
        }
        fl3 tomove;
        if(window->isKeyDown(KEY_W)) {
            tomove.z -= 1.f;
        }
        if(window->isKeyDown(KEY_S)) {
            tomove.z += 1.f;
        }
        if(window->isKeyDown(KEY_A)) {
            tomove.x -= 1.f;
        }
        if(window->isKeyDown(KEY_D)) {
            tomove.x += 1.f;
        }
        if(window->isKeyDown(KEY_SPACE)) {
            tomove.y += 1.f;
        }
        if(window->isKeyDown(KEY_CTRL)) {
            tomove.y -= 1.f;
        }
        tomove.normalize();
        tomove *= MOVESPEED * window->getdtBetweenUpdates();
        cam.move(tomove);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // now do GL stuff

        // g-buffer pass
        {
            gbufs->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            dprepass.use();
            cam.toMatrixAll(mats);
            mats.matrixToUniform(MatrixStack::MODELVIEW);
            mats.matrixToUniform(MatrixStack::PROJECTION);
            testobj.draw(dprepass);
            mats.pushMatrix(MatrixStack::MODELVIEW);
            mats.translate(10, 0, 0);
            mats.matrixToUniform(MatrixStack::MODELVIEW);
            testobj.draw(dprepass);
            mats.popMatrix(MatrixStack::MODELVIEW);
            // g-buffers should be populated now
            glUseProgram(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // render pass
        {
            fbuf->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // we have to use image units with everything because compute shader has trouble with texelfetch
//            glActiveTexture(GL_TEXTURE0);
//            gbufs->bindColorTexture(0);
//            glActiveTexture(GL_TEXTURE1);
//            gbufs->bindColorTexture(1);
//            glActiveTexture(GL_TEXTURE2);
//            gbufs->bindColorTexture(2);
//            glActiveTexture(GL_TEXTURE3);
//            gbufs->bindColorTexture(3);
//            glActiveTexture(GL_TEXTURE4);
//            gbufs->bindDepthTexture();
//            smapRender.bind(4);
//            glActiveTexture(GL_TEXTURE5);
//            glBindTexture(GL_TEXTURE_BUFFER, lightsTex);

            glBindImageTexture(0, fbuf->getColorID(0), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glBindImageTexture(1, fbuf->getDepthID(), 0, false, 0, GL_WRITE_ONLY, GL_R32F);
            glBindImageTexture(2, gbufs->getColorID(0), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(3, gbufs->getColorID(1), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(4, gbufs->getColorID(2), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(5, gbufs->getColorID(3), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(6, gbufs->getColorID(4), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(7, lightsTex, 0, false, 0, GL_READ_ONLY, GL_R32F);

            tileCompute.use();
            glUniform1ui(5, lights.size());

            glUniform3fv(6, 1, &lights[0].pos.x);
            glUniform3fv(7, 1, &lights[0].power.x);
            glUniform1f(8, lights[0].size);

            glUniformMatrix4fv(3, 1, false, camMv.data());
            glUniformMatrix4fv(4, 1, false, camPj.data());
            glDispatchCompute(numTilesX, numTilesY, 1);
            glUseProgram(0);
            // we plan to use compute shader output images as texture and framebuffer
            //glMemoryBarrier(GL_ALL_BARRIER_BITS);
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);

            glBindImageTexture(0, 0, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, 0, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(2, 0, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(3, 0, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(4, 0, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(5, 0, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(6, 0, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(7, 0, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);

//            glBindTexture(GL_TEXTURE_BUFFER, 0);
//            glActiveTexture(GL_TEXTURE4);
//            glBindSampler(4,0);
//            glBindTexture(GL_TEXTURE_2D, 0);
//            glActiveTexture(GL_TEXTURE3);
//            glBindTexture(GL_TEXTURE_2D, 0);
//            glActiveTexture(GL_TEXTURE2);
//            glBindTexture(GL_TEXTURE_2D, 0);
//            glActiveTexture(GL_TEXTURE1);
//            glBindTexture(GL_TEXTURE_2D, 0);
//            glActiveTexture(GL_TEXTURE0);
//            glBindTexture(GL_TEXTURE_2D, 0);
        }

        {
            // add the markers for lights
            glEnable(GL_DEPTH_TEST);
            fbuf->bind();
            lightRender.use();
            mats.loadIdentity(MatrixStack::MODELVIEW);
            mats.loadIdentity(MatrixStack::PROJECTION);
            cam.toMatrixAll(mats);
            mats.matrixToUniform(MatrixStack::PROJECTION);
            for (unsigned int i = 0; i < lights.size(); i++) {
                Light &l = lights.at(i);
                mats.pushMatrix(MatrixStack::MODELVIEW);
                mats.translate(l.pos);
                //mats.scale(l.size, l.size, l.size);
                mats.matrixToUniform(MatrixStack::MODELVIEW);
                glUniform1f(2, (i==closestLight) ? 1.0f : 0);
                glUniform3fv(3, 1, &l.power.x);
                lightMesh->draw();
                mats.popMatrix(MatrixStack::MODELVIEW);
            }
            glUseProgram(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        fbuf->blit(true, false); // this should take care of SSAA resolve
        window->finishFrame();
        Sleep(1);
    }

    return 0;
}
