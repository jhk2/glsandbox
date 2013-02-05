#include "winglbase.h"
#include "glextensionfuncs.h"
#include "matrixstack.h"
#include "shader.h"
#include "mesh.h"
#include <stdio.h>
#include "debug.h"
#include "texture.h"
#include "obj.h"
#include "camera.h"
#include "framebuffer.h"
#include "utils.h"

#define MOUSE_SENSITIVITY 20.f
#define MOVESPEED 0.2f

MatrixStack mats;
Camera cam;
Framebuffer *fbuf;
Framebuffer *msbuf;

long OnResize(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	//~ printf("resize\n"); fflush(stdout);
	// lparam loword has width, hiword has height
	wnd.resize(LOWORD(lparam), HIWORD(lparam));
	if(fbuf) {
		fbuf->resize(wnd.getWidth(), wnd.getHeight());
	}
	if(msbuf) {
		msbuf->resize(wnd.getWidth(), wnd.getHeight());
	}
	// change any camera related stuff here
	//~ mats.loadIdentity(MatrixStack::MODELVIEW);
	//~ mats.loadIdentity(MatrixStack::PROJECTION);
	//~ mats.ortho(0, 1, 0, 1);
	//mats.perspective(45, ((float) wnd.getWidth() / (float) wnd.getHeight()), 0.1f, 100.f);
	cam.init(45, ((float) wnd.getWidth() / (float) wnd.getHeight()), 0.1f, 100.f);
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	fbuf = 0; msbuf = 0;
	WinGLBase* window = new WinGLBase(hInstance, 800, 600);
	window->addMessageHandler(WM_SIZE, OnResize);
	window->showWindow(nShowCmd);
	cam.setPos(fl3(0, 10, 10));
	printf("make shader\n"); fflush(stdout);
	Shader shader ("shader.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
	Shader texshader ("tex.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
	//~ glBindFragDataLocation(shader.getProgramID(), 0, "out_Color");
	// get uniform locations
	GLint mvloc = shader.getUniformLocation("mvMatrix");
	GLint pjloc = shader.getUniformLocation("pjMatrix");
	checkGLError("getuniformlocation");
	
	printf("init matrix uniform locs at %i, %i\n", mvloc, pjloc);
	mats.initUniformLocs(mvloc, pjloc);
	
	printf("init framebuffer\n"); fflush(stdout);
	FramebufferParams params;
	params.width = window->getWidth();
	params.height = window->getHeight();
	params.numSamples = 0;
	params.numMrts = 1;
	params.depthEnable = true;
	params.format = GL_RGBA32F;
	params.depthFormat = GL_DEPTH_COMPONENT32F;
	params.type = GL_TEXTURE_2D;
	params.filter = GL_LINEAR;
	fbuf = new Framebuffer(params);
	
	params.numSamples = 4;
	msbuf = new Framebuffer(params);
	
	printf("init quad data\n"); fflush(stdout);
	// let's make a simple quad for a mesh
	//~ std::vector<std::pair<unsigned int, AttributeInfo*>> attribInfo;
	//~ printf("attribinfo vector created\n"); fflush(stdout);
	//~ attribInfo.push_back(std::pair<unsigned int, AttributeInfo*>(0, new AttributeInfoSpec<GLfloat>(GL_FLOAT, 3)));
	printf("making vert vector\n"); fflush(stdout);
	std::vector<PTvert> verts;
	PTvert v;
	v.pos_ = fl3(0,0,0); v.tex_ = fl3(0,0,0); verts.push_back(v);
	v.pos_ = fl3(1,0,0); v.tex_ = fl3(1,0,0); verts.push_back(v);
	v.pos_ = fl3(1,1,0); v.tex_ = fl3(1,1,0); verts.push_back(v);
	v.pos_ = fl3(0,1,0); v.tex_ = fl3(0,1,0); verts.push_back(v);
	//~ verts.push_back(fl3(0, 0, 0));
	//~ verts.push_back(fl3(1, 0, 0));
	//~ verts.push_back(fl3(1, 1, 0));
	//~ verts.push_back(fl3(0, 1, 0));
	
	printf("verts vector created\n"); fflush(stdout);
	//~ VertexBufferSpec<fl3> vbuf (&verts);
	std::vector<unsigned char> inds;
	inds.push_back(0x0); inds.push_back(0x1); inds.push_back(0x2); inds.push_back(0x3);
	printf("make mesh from quad data\n"); fflush(stdout);
	
	Mesh<PTvert, GLubyte> quad (GL_QUADS);
	quad.addAttrib(0, AttributeInfoSpec<GLfloat>(3)).addAttrib(1, AttributeInfoSpec<GLfloat>(3)).addVerts(verts).addInds(inds).finalize();
	
	// make a quad for the ground
	std::vector<PTNvert> gverts;
	PTNvert gv;
	gv.norm_ = fl3(0, 1.0f, 0);
	gv.pos_ = fl3(-0.5, 0, -0.5); gv.tex_ = fl3(0, 0, 0); gverts.push_back(gv);
	gv.pos_ = fl3(-0.5, 0, 0.5); gv.tex_ = fl3(1, 0, 0); gverts.push_back(gv);
	gv.pos_ = fl3(0.5, 0, 0.5); gv.tex_ = fl3(1, 1, 0); gverts.push_back(gv);
	gv.pos_ = fl3(0.5, 0, -0.5); gv.tex_ = fl3(0, 1, 0); gverts.push_back(gv);
	
	Mesh<PTNvert, GLubyte> gquad (GL_QUADS);
	gquad.addAttrib(0, AttributeInfoSpec<GLfloat>(3)).addAttrib(1, AttributeInfoSpec<GLfloat>(3)).addAttrib(2, AttributeInfoSpec<GLfloat>(3)).addVerts(gverts).addInds(inds).finalize();
	
	Obj testobj ("../assets/ServerBot1.obj");
	fl3 objmin, objmax;
	testobj.getBounds(objmin, objmax);
	printf("obj bounds are %f,%f,%f to %f,%f,%f\n", objmin.x, objmin.y, objmin.z, objmax.x, objmax.y, objmax.z); fflush(stdout);
	
	Texture tex ("../assets/Jellyfish.jpg");
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0,0,0,0);
	
	while(window->isActive()) {
		// always call at the beginning of loop iteration (should we combine with isActive and finishFrame?)
		window->update();
		
		if(window->isKeyPress(KEY_ESC)) {
			window->close();
			break;
		}
		if(window->isMousePress(MOUSE_RIGHT)) {
			window->hideCursor();
			window->holdCursor(true);
		} else if(window->isMouseRelease(MOUSE_RIGHT)) {
			window->showCursor();
			window->holdCursor(false);
		}
		if(window->isMouseDown(MOUSE_RIGHT)) {
			fl2 dxdy = window->getMouseNorm_dxdy();
			// dxdy is with the origin in the top left of the window
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
		tomove *= MOVESPEED;
		cam.move(tomove);
		
		// start GL code
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//~ fbuf->bind();
		msbuf->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		cam.toMatrixAll(mats);
		
		glActiveTexture(GL_TEXTURE0);
		tex.bind();
		texshader.use();
		mats.initUniformLocs(texshader.getUniformLocation("mvMatrix"), texshader.getUniformLocation("pjMatrix"));
		mats.pushMatrix(MatrixStack::MODELVIEW);
		mats.scale(100, 0, 100);
		mats.matrixToUniform(MatrixStack::MODELVIEW);
		mats.matrixToUniform(MatrixStack::PROJECTION);
		gquad.draw();
		mats.popMatrix(MatrixStack::MODELVIEW);
		glUseProgram(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		mats.initUniformLocs(shader.getUniformLocation("mvMatrix"), shader.getUniformLocation("pjMatrix"));
		mats.matrixToUniform(MatrixStack::MODELVIEW);
		mats.matrixToUniform(MatrixStack::PROJECTION);
		testobj.draw(shader, mats);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		msbuf->blit(*fbuf);
		
		glDisable(GL_DEPTH_TEST);
		fbuf->bindColorTexture();
		texshader.use();
		mats.loadIdentity(MatrixStack::MODELVIEW);
		mats.loadIdentity(MatrixStack::PROJECTION);
		mats.ortho(0, 1, 0, 1);
		mats.initUniformLocs(texshader.getUniformLocation("mvMatrix"), texshader.getUniformLocation("pjMatrix"));
		mats.matrixToUniform(MatrixStack::MODELVIEW);
		mats.matrixToUniform(MatrixStack::PROJECTION);
		quad.draw();
		glUseProgram(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		// end GL code
		window->finishFrame();
	}
	delete fbuf;
	return 0;
}