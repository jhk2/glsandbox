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
    //glEnable(GL_DEBUG_OUTPUT);
	//glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    cam.setPos(fl3(0, 10, 10));

    ShaderProgram dprepass ("deferredprepass.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
    ShaderProgram drender ("deferredrender.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
    ShaderProgram lightRender ("light.glsl", Shader::VERTEX_SHADER | Shader::TESSELLATION_SHADER | Shader::GEOMETRY_SHADER | Shader::FRAGMENT_SHADER);

    mats.initUniformLocs(0,1);

    FramebufferParams params;
    params.width = window->getWidth();
    params.height = window->getHeight();
    params.numSamples = 4;
    params.numMrts = 1;
    params.colorEnable = true;
    params.depthEnable = true;
    params.format = GL_RGBA32F;
    params.depthFormat = GL_DEPTH_COMPONENT32F;
    params.type = GL_TEXTURE_2D_MULTISAMPLE;
    params.filter = GL_LINEAR;
    fbuf = new Framebuffer(params);

    // g buffers we need
    // ambient color
    // diffuse albedo
    // specular color
    // normals
    params.numMrts = 4;
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
        l.size = 1.0;
        lights.push_back(l);
        l.pos = fl3(-10, 10, 10);
        lights.push_back(l);

        // for the shaders, we want to store the light properties in a texture buffer
        glGenBuffers(1, &lightsBuf);
        glBindBuffer(GL_TEXTURE_BUFFER, lightsBuf);
        glBufferData(GL_TEXTURE_BUFFER, sizeof(Light)*lights.size(), &lights[0], GL_STATIC_READ);
        glGenTextures(1, &lightsTex);
        glBindTexture(GL_TEXTURE_BUFFER, lightsTex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, lightsBuf);
        // NSIGHT doesn't support this
		//glTexBufferRange(GL_TEXTURE_BUFFER, GL_R32F, lightsBuf, 0, sizeof(Light)*lights.size());
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
        glBindTexture(GL_TEXTURE_BUFFER, 0);
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
            // now calculate the ray params according to relative FOV
            // this is slightly inaccurate, switching to matrix operations
            const float FOV_X = FOV_Y * window->getWidth() / (float) window->getHeight();
            /*
            fl3 ray;
            float cosfovx = cos(0.5 * mpos.x * DEGTORAD(FOV_X));
            ray.x = sin(0.5 * mpos.x * DEGTORAD(FOV_X));
            ray.y = sin(0.5 * mpos.y * DEGTORAD(FOV_Y)) / cos(0.5 * mpos.y * DEGTORAD(FOV_Y));
            ray.z = -cosfovx;
            ray.normalize();
            printf("ray1: %g, %g, %g\n", ray.x, ray.y, ray.z); fflush(stdout);
            */

            // see http://antongerdelan.net/opengl/raycasting.html
            // transform to homogeneous clip coordinates
            //float ray_clip [4];
            //ray_clip[0] = mpos.x; ray_clip[1] = mpos.y; ray_clip[2] = -1.0f; ray_clip[3] = 1.0f;
            fl3 ray_clip;
            ray_clip.x = mpos.x; ray_clip.y = mpos.y; ray_clip.z = -1.0f;
            // transform to camera/eye space
            Matrix invproj;
            camPj.getInverse(invproj);
            fl3 ray = invproj.multiplyPoint(ray_clip);
            ray.normalize();
            //printf("ray2: %g, %g, %g\n", ray.x, ray.y, ray.z); fflush(stdout);

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
                    const float test = (loc*loc - fl3::dot(oc, oc) + li.size*li.size);

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
                    /*
                    // the math for this isn't working right, but leaving it here in case of revisit in the future
                    // need to calculate the screen space extents of the light
                    // start with the center point of the light, and find its screen space position
                    cam.toMatrixAll(mats);
                    mats.copy(MatrixStack::MODELVIEW, camMv);
                    fl3 sspos = camMv.multiplyPoint(li.pos);
                    const float z = sspos.z;
                    mats.copy(MatrixStack::PROJECTION, camMv);
                    sspos = camMv.multiplyPoint(sspos);
                    // transform to normalized window coordinates by negating y, adding 1 and scaling by 0.5
                    sspos.y *= -1.f;
                    sspos += fl3(1,1,0);
                    sspos *= fl3(0.5f, 0.5f, 0);
                    //printf("light position is %g, %g\n", sspos.x, sspos.y);
                    //printf("mouse position is %g, %g\n", window->getMouseNormPos().x, window->getMouseNormPos().y);
                    //fflush(stdout);

                    // now to find light's extents onscreen at the given distance from camera
                    // we define the vertical FOV
                    // then we know the total viewable range at the given distance
                    float totalview = 2 * -z * tan(0.5 * RADTODEG(FOV_Y));
                    // then we divide the radius by that total amount to find the fraction of the screen taken up by that radius
                    float rscaled = li.size / totalview;

                    // then calculate the distance from the mouse pointer to the light center
                    const fl2 &mpos = window->getMouseNormPos();
                    const fl3 mtol = sspos - fl3(mpos.x, mpos.y, 0);
                    const float mdist = mtol.length();

                    printf("mouse distance to light: %g, scaled radius: %g\n", mdist, rscaled);
                    fflush(stdout);

                    //printf("light radius in y pixels (%u): %g\n", window->getHeight(), rscaled * window->getHeight());
                    //fflush(stdout);
                    */
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

                /* switching from dxdy implementation to matrix backprojection method
                const fl2 dxdy = window->getMouseNorm_dxdy();
                // we want to move y along the up vector and move x along our generated ray cross up vector
                const fl3 ytrans = cam.getUp();
                const fl3 xtrans = fl3::cross(cam.getLook(), ytrans);
                Light &li = lights.at(closestLight);
                //printf("moving light index %i, dxdy=%g,%g\n", closestLight, dxdy.x, dxdy.y);
                // TODO: up vector is incorrect when looking in from the side
                //printf("xtrans: %g,%g,%g; ytrans: %g,%g,%g\n", xtrans.x, xtrans.y, xtrans.z, ytrans.x, ytrans.y, ytrans.z);
                //printf("position before: %g,%g,%g\n", li.pos.x, li.pos.y, li.pos.z);
                const float scalex = 2 * closestDepth * tan(DEGTORAD(FOV_X/2.0f));
                const float scaley = 2 * closestDepth * tan(DEGTORAD(FOV_Y/2.0f));
                li.pos += (xtrans * dxdy.x * scalex) + (ytrans * -dxdy.y * scaley);
                //printf("position after: %g,%g,%g\n", li.pos.x, li.pos.y, li.pos.z);
                fflush(stdout);
                */
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
        }

        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // render pass
        {
            fbuf->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            mats.loadIdentity(MatrixStack::MODELVIEW);
            mats.loadIdentity(MatrixStack::PROJECTION);
            mats.ortho(0, 1, 0, 1);
            glActiveTexture(GL_TEXTURE0);
            gbufs->bindColorTexture(0);
            glActiveTexture(GL_TEXTURE1);
            gbufs->bindColorTexture(1);
            glActiveTexture(GL_TEXTURE2);
            gbufs->bindColorTexture(2);
            glActiveTexture(GL_TEXTURE3);
            gbufs->bindColorTexture(3);
            glActiveTexture(GL_TEXTURE4);
            gbufs->bindDepthTexture();
            smapRender.bind(4);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_BUFFER, lightsTex);

            drender.use();
            glUniform1ui(5, lights.size());
            mats.matrixToUniform(MatrixStack::MODELVIEW);
            mats.matrixToUniform(MatrixStack::PROJECTION);
            glUniformMatrix4fv(3, 1, false, camMv.data());
            glUniformMatrix4fv(4, 1, false, camPj.data());
            quad.draw();

            glBindTexture(GL_TEXTURE_BUFFER, 0);
            glActiveTexture(GL_TEXTURE4);
            glBindSampler(0,0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);

            // another way to transfer depth data
            //gbufs->blit(*fbuf, false, true);
            //fbuf->bind();

            glEnable(GL_DEPTH_TEST);
            // add the markers for lights
            lightRender.use();
            mats.loadIdentity(MatrixStack::MODELVIEW);
            mats.loadIdentity(MatrixStack::PROJECTION);
            cam.toMatrixAll(mats);
            mats.matrixToUniform(MatrixStack::PROJECTION);
            for (unsigned int i = 0; i < lights.size(); i++) {
                Light &l = lights.at(i);
                mats.pushMatrix(MatrixStack::MODELVIEW);
                mats.translate(l.pos);
                mats.scale(l.size, l.size, l.size);
                mats.matrixToUniform(MatrixStack::MODELVIEW);
                glUniform1f(2, (i==closestLight) ? 1.0f : 0);
                lightMesh->draw();
                mats.popMatrix(MatrixStack::MODELVIEW);
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        fbuf->blit(true, false); // this should take care of SSAA resolve
        window->finishFrame();
        Sleep(1);
    }

    return 0;
}
