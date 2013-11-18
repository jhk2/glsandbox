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
Matrix lightMat;
FirstPersonCamera cam;
Framebuffer *fbuf; // final non-aa framebuffer
Framebuffer *gbufs; // g-buffers for deferred
WinGLBase *window;

struct Light {
    fl3 pos;
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
    glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    cam.setPos(fl3(0, 10, 10));

    ShaderProgram dprepass ("deferredprepass.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
    ShaderProgram drender ("deferredrender.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
    ShaderProgram lightRender ("light.glsl", Shader::VERTEX_SHADER | Shader::TESSELLATION_SHADER | Shader::FRAGMENT_SHADER);

    mats.initUniformLocs(0,1);

    FramebufferParams params;
    params.width = window->getWidth();
    params.height = window->getHeight();
    params.numSamples = 0;
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
    params.numMrts = 4;
    gbufs = new Framebuffer(params);

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

    Obj testobj ("../assets/ServerBot1.obj");

    Mesh<GLubyte> *lightMesh = createIcosahedron(true);
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    // populate some lights
    {
        Light l;
        l.pos = fl3(10, 10, 10);
        l.size = 1.0;
        lights.push_back(l);
    }

    Texture tex ("../assets/white.png");

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
        if (window->isMousePress(MOUSE_LEFT)) {
            cam.toMatrixMv(lightMat);
            // generate a ray in normalized device coordinates
            fl2 mpos = window->getMouseNormPos();
            // transform to ndc
            mpos *= 2;
            mpos -= fl2(1,1);
            mpos.y *= -1;
            // now calculate the ray params according to relative FOV
            const float FOV_X = FOV_Y * window->getWidth() / (float) window->getHeight();
            fl3 ray;
            float cosfovx = cos(0.5 * mpos.x * DEGTORAD(FOV_X));
            ray.x = sin(0.5 * mpos.x * DEGTORAD(FOV_X));
            ray.y = sin(0.5 * mpos.y * DEGTORAD(FOV_Y)) / cos(0.5 * mpos.y * DEGTORAD(FOV_Y));
            ray.z = -cosfovx;
            ray.normalize();
            // see if we select a light
            for (unsigned int i = 0; i < lights.size(); i++) {
                Light &li = lights.at(i);

                // do a ray-sphere intersection for the light
                // transform light center to view space

                fl3 wpos = lightMat.multiplyPoint(li.pos);

                // ray-sphere intersection formula
                const fl3 oc = -wpos;
                const float loc = fl3::dot(ray, oc);
                const float test = (loc*loc - fl3::dot(oc, oc) + li.size*li.size);

                if (test >= 0) {
                    // we have an intersection
                    printf("ray intersects sphere\n");
                } else {
                    printf("ray does not intersect sphere\n");
                }
                fflush(stdout);
                /*
                // the math for this isn't working right, but leaving it here in case of revisit in the future
                // need to calculate the screen space extents of the light
                // start with the center point of the light, and find its screen space position
                cam.toMatrixAll(mats);
                mats.copy(MatrixStack::MODELVIEW, lightMat);
                fl3 sspos = lightMat.multiplyPoint(li.pos);
                const float z = sspos.z;
                mats.copy(MatrixStack::PROJECTION, lightMat);
                sspos = lightMat.multiplyPoint(sspos);
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
                break;
            }
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
        {
            // g-buffer pass
            gbufs->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            dprepass.use();
            cam.toMatrixAll(mats);
            mats.matrixToUniform(MatrixStack::MODELVIEW);
            mats.matrixToUniform(MatrixStack::PROJECTION);
            testobj.draw(dprepass);
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

            drender.use();
            mats.matrixToUniform(MatrixStack::MODELVIEW);
            mats.matrixToUniform(MatrixStack::PROJECTION);
            quad.draw();

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

            // add the markers for lights
            lightRender.use();
            mats.loadIdentity(MatrixStack::MODELVIEW);
            mats.loadIdentity(MatrixStack::PROJECTION);
            cam.toMatrixAll(mats);
            for (unsigned int i = 0; i < lights.size(); i++) {
                Light &l = lights.at(i);
                mats.pushMatrix(MatrixStack::MODELVIEW);
                mats.translate(l.pos);
                mats.scale(l.size, l.size, l.size);
                mats.matrixToUniform(MatrixStack::MODELVIEW);
                mats.matrixToUniform(MatrixStack::PROJECTION);
                lightMesh->draw();
                mats.popMatrix(MatrixStack::MODELVIEW);
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        fbuf->blit(true, false);
        window->finishFrame();
        Sleep(1);
    }

    return 0;
}
