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
#include "sampler.h"

#define MOUSE_SENSITIVITY 20.f
#define MOVESPEED 0.05f

MatrixStack mats;
Matrix eyePj;
FirstPersonCamera cam;
Framebuffer *fbuf; // final nonmsaa framebuffer
Framebuffer *msbuf; // msaa framebuffer for depth prepass
Framebuffer *aobuf; // msaa framebuffer for calculating AO
Framebuffer *filterbuf; // msaa framebuffer for filtering AO results
WinGLBase *window;

long OnResize(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam)
{
    wnd.resize(LOWORD(lparam), HIWORD(lparam));
    if (fbuf) {
        fbuf->resize(wnd.getWidth(), wnd.getHeight());
    }
    cam.init(45, wnd.getAspect(), 1.f, 500.f);
    glViewport(0, 0, wnd.getWidth(), wnd.getHeight());
    return 0;
}

float genRand()
{
    return (rand() % 2 == 0 ? 1.0f : -1.0f) * ((float) rand() / RAND_MAX);
}

void genSamples()
{
    for (unsigned int i = 0; i < 16;) {
        /*
        // sampling disc generation code (rejection sampling)
        const fl2 vec = fl2(genRand(), genRand());
        if (vec.lengthSq() <= 1.0f) {
            printf("vec2(%f, %f), \n", vec.x, vec.y);
            i++;
        }
        */
        const fl3 vec = fl3(genRand(), genRand(), genRand());
        if (vec.lengthSq() <= 1.0f) {
            printf("vec3(%f, %f, %f), \n", vec.x, vec.y, vec.z);
            i++;
        }
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    fbuf = 0;
    window = new WinGLBase(hInstance, 1024, 768);
    window->addMessageHandler(WM_SIZE, OnResize);
    window->showWindow(nShowCmd);

    glDebugMessageCallbackARB(errorCallback, NULL);
//    glEnable(GL_DEBUG_OUTPUT);
//    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    cam.setPos(fl3(0, 10, 10));

    ShaderProgram render ("render.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
    ShaderProgram ssao ("ssao.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
    ShaderProgram fast ("fastssao.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
    ShaderProgram prepass ("prepass.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
    ShaderProgram filter ("filter.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);

    mats.initUniformLocs(0, 1); // modelview at location 0, projection at 1

    FramebufferParams params;
    params.width = window->getWidth();
    params.height = window->getHeight();
    params.numSamples = 0;
    params.numMrts = 1;
    params.colorEnable = true;
    params.depthEnable = false;
    params.format = GL_RGBA32F;
    params.type = GL_TEXTURE_2D;
    params.filter = GL_LINEAR;
    fbuf = new Framebuffer(params);

    msbuf = new Framebuffer(params);

    params.format = GL_R32F;
    filterbuf = new Framebuffer(params);

    params.colorEnable = true;
    params.depthEnable = true;
    params.format = GL_RGB32F; // view space position and normals
    params.numMrts = 2;
    params.depthFormat = GL_DEPTH_COMPONENT32F;
    params.filter = GL_NEAREST;
    aobuf = new Framebuffer(params);

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

    Texture tex ("../assets/white.png");

    Sampler depthSampler;
    glSamplerParameteri(depthSampler.getID(), GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glSamplerParameteri(depthSampler.getID(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(depthSampler.getID(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

        {
            aobuf->bind();
            // depth + normal pre-pass
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            prepass.use();
            cam.toMatrixAll(mats);
            mats.matrixToUniform(MatrixStack::MODELVIEW);
            mats.matrixToUniform(MatrixStack::PROJECTION);
            gquad.draw();
            testobj.draw(prepass);

//            filterbuf->bind();
//            glClear(GL_COLOR_BUFFER_BIT);
//            glDisable(GL_DEPTH_TEST);
//            filter.use();

//            glActiveTexture(GL_TEXTURE0);
//            aobuf->bindColorTexture(0);

            mats.copy(MatrixStack::PROJECTION, eyePj);
            mats.loadIdentity(MatrixStack::MODELVIEW);
            mats.ortho(0, 1, 0, 1);
//            mats.matrixToUniform(MatrixStack::MODELVIEW);
//            mats.matrixToUniform(MatrixStack::PROJECTION);
//            quad.draw();
        }
        glBindSampler(0, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        {
            glCullFace(GL_BACK);
            msbuf->bind();
            glClear(GL_COLOR_BUFFER_BIT);
            glActiveTexture(GL_TEXTURE0);
//            filterbuf->bindColorTexture();
            // use positions for deferred approach
            aobuf->bindColorTexture(0); // positions
            glActiveTexture(GL_TEXTURE1);
            aobuf->bindColorTexture(1); // normals
            glActiveTexture(GL_TEXTURE2);
            // use depth buffer for other approach
            aobuf->bindDepthTexture();
            depthSampler.bind(2);

            //ssao.use();
            fast.use();

            mats.matrixToUniform(MatrixStack::MODELVIEW);
            mats.matrixToUniform(MatrixStack::PROJECTION);
            glUniformMatrix4fv(2, 1, false, eyePj.data()); // bind to uniform loc 2

            quad.draw();
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        msbuf->blit(*fbuf);
        fbuf->blit();

        window->finishFrame();
        Sleep(1);
    }

    return 0;
}
