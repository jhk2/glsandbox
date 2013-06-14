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
#include "constants.h"
#include "sampler.h"

#define MOUSE_SENSITIVITY 20.f
#define MOVESPEED 0.05f

MatrixStack mats;
MatrixStack lmats;
FirstPersonCamera cam;
Framebuffer *fbuf;
Framebuffer *shadowmap;
WinGLBase *window;

struct SpotLight {
	SpotLight(fl3 &pos) : pos_(pos), fovy_(45), aspect_(1.0f), near_(20.0f), far_(100.0f), mesh_(GL_LINES)
	{
		init();
	}
	// drawing from the light's pov (for shadow map generation)
	void multPovMatrix(MatrixStack &mstack)
	{
		mstack.perspective(fovy_, aspect_, near_, far_);
		mstack.lookAt(pos_.x, pos_.y, pos_.z, 0, 0, 0, 0, 1, 0);
    }
	// for transforming the frustum to be drawn on screen to visualize the light
	void multDrawMatrix(MatrixStack &mstack)
	{
		if (pos_.isZero()) return;
		// calculate rotation
		fl3 start (0,0,-1);
		fl3 look = -pos_; look.normalize();
		if (start == look) return;
		float pitch = asin(look.y);
		pitch = RADTODEG(pitch);
		look.y = 0;
		look.normalize();
		float yaw = acos(fl3::dot(start, look));
		yaw = RADTODEG(yaw);
		fl3 axis = fl3::cross(start, look);
		if (axis.y < 0) {
			yaw = -yaw;
		}
		mstack.translate(pos_.x, pos_.y, pos_.z);
		mstack.rotate(yaw, 0, 1, 0);
		mstack.rotate(pitch, 1, 0, 0);
	}
	void init()
	{
		std::vector<GLubyte> inds;
		// generate the geometry for the frustum
		// color-wise we'll make volume edges white, near plane red, far plane green
		PTvert origin;
		origin.pos_ = fl3(0,0,0);
		origin.tex_ = fl3(1,1,1);
		mesh_.addVert(origin);
		
		float xbase = aspect_ * tan(DEGTORAD(fovy_/2.0f));
		float ybase = tan(DEGTORAD(fovy_/2.0f));
		
		// far plane coordinates, first for the frustum edges
		PTvert v;
		float farx = far_ * xbase;
		float fary = far_ * ybase;
		v.pos_ = fl3(farx, fary, -far_); v.tex_ = fl3(1,1,1);
		mesh_.addVert(v);
		
		v.pos_ = fl3(-farx, fary, -far_); mesh_.addVert(v);
		v.pos_ = fl3(-farx, -fary, -far_); mesh_.addVert(v);
		v.pos_ = fl3(farx, -fary, -far_); mesh_.addVert(v);
		// add indices, origin is idx 0, far plane is [1,4]
		for (GLubyte i = 0x1; i <= 0x4; i++) {
			inds.push_back(0x0); inds.push_back(i);
		}
		
		// now the far plane itself
		v.pos_ = fl3(farx, fary, -far_);
		v.tex_ = fl3(0,1,0);
		mesh_.addVert(v);
		v.pos_ = fl3(-farx, fary, -far_); mesh_.addVert(v);
		v.pos_ = fl3(-farx, -fary, -far_); mesh_.addVert(v);
		v.pos_ = fl3(farx, -fary, -far_); mesh_.addVert(v);
		// plane vertices are idx [5,8]
		inds.push_back(0x5); inds.push_back(0x6);
		inds.push_back(0x6); inds.push_back(0x7);
		inds.push_back(0x7); inds.push_back(0x8);
		inds.push_back(0x8); inds.push_back(0x5);
		
		// near plane
		float nearx = near_ * xbase;
		float neary = near_ * ybase;
		v.pos_ = fl3(nearx, neary, -near_); v.tex_ = fl3(1,0,0); mesh_.addVert(v);
		v.pos_ = fl3(-nearx, neary, -near_); mesh_.addVert(v);
		v.pos_ = fl3(-nearx, -neary, -near_); mesh_.addVert(v);
		v.pos_ = fl3(nearx, -neary, -near_); mesh_.addVert(v);
		// plane vertices are idx [9,12]
		inds.push_back(0x9); inds.push_back(0xa);
		inds.push_back(0xa); inds.push_back(0xb);
		inds.push_back(0xb); inds.push_back(0xc);
		inds.push_back(0xc); inds.push_back(0x9);
		
		printf("finalizing frustum\n"); fflush(stdout);
		mesh_.addAttrib(0, AttributeInfoSpec<GLfloat>(3)).addAttrib(3, AttributeInfoSpec<GLfloat>(3)).addInds(inds).finalize();
	};
	fl3 pos_;
	// for now it will always point at the origin
	float fovy_;
	float aspect_;
	float near_;
	float far_;
	// we'll substitute vertex colors for Tex and make it vertex attribute 3
	InterleavedMesh<PTvert, GLubyte> mesh_;
};

long OnResize(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	//~ printf("resize\n"); fflush(stdout);
	// lparam loword has width, hiword has height
	wnd.resize(LOWORD(lparam), HIWORD(lparam));
	if(fbuf) {
		fbuf->resize(wnd.getWidth(), wnd.getHeight());
	}
	// change any camera related stuff here
	//~ mats.loadIdentity(MatrixStack::MODELVIEW);
	//~ mats.loadIdentity(MatrixStack::PROJECTION);
	//~ mats.ortho(0, 1, 0, 1);
	//mats.perspective(45, ((float) wnd.getWidth() / (float) wnd.getHeight()), 0.1f, 100.f);
	cam.init(45, wnd.getAspect(), 1.f, 500.f);
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	fbuf = 0;
	window = new WinGLBase(hInstance, 1024, 768);
	window->addMessageHandler(WM_SIZE, OnResize);
	window->showWindow(nShowCmd);
	
	glDebugMessageCallbackARB(errorCallback, NULL);
	glEnable(GL_DEBUG_OUTPUT);
	
	cam.setPos(fl3(0, 10, 10));
	printf("make shader\n"); fflush(stdout);
	ShaderProgram shader ("shader.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
	ShaderProgram texshader ("tex.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
	ShaderProgram colshader ("color.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
    ShaderProgram shadowshader("pcss.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
//    ShaderProgram shadowshader("esm.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
	ShaderProgram shadowgen("shadowgen.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
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
	params.colorEnable = true;
	params.depthEnable = true;
	params.format = GL_RGBA32F;
	params.depthFormat = GL_DEPTH_COMPONENT32F;
	params.type = GL_TEXTURE_2D;
	params.filter = GL_LINEAR;
	fbuf = new Framebuffer(params);
	params.width = 1024;
	params.height = 1024;
	params.filter = GL_NEAREST;
    params.colorEnable = false;
	shadowmap = new Framebuffer(params);
	
	Sampler smapRender;
	glSamplerParameteri(smapRender.getID(), GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glSamplerParameteri(smapRender.getID(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(smapRender.getID(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(smapRender.getID(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(smapRender.getID(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
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
	
	printf("verts vector created\n"); fflush(stdout);
	std::vector<GLubyte> inds;
	inds.push_back(0x0); inds.push_back(0x1); inds.push_back(0x2); inds.push_back(0x3);
	printf("make mesh from quad data\n"); fflush(stdout);
	
	InterleavedMesh<PTvert, GLubyte> quad (GL_QUADS);
	quad.addAttrib(0, AttributeInfoSpec<GLfloat>(3)).addAttrib(1, AttributeInfoSpec<GLfloat>(3));
	quad.addVerts(verts).addInds(inds).finalize();
	
	// make a quad for the ground
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
	fl3 objmin, objmax;
	testobj.getBounds(objmin, objmax);
	printf("obj bounds are %f,%f,%f to %f,%f,%f\n", objmin.x, objmin.y, objmin.z, objmax.x, objmax.y, objmax.z); fflush(stdout);
	
	Texture tex ("../assets/white.png");
	
	// make the spot light
	SpotLight light (fl3(40, 40, 40));
	glLineWidth(4);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0,0,0,0);

//    glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
//    glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
//    glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
	
	float timer = 0.0f;
	
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
		tomove *= MOVESPEED * window->getdtBetweenUpdates();
		cam.move(tomove);
		
		// move the light
		timer += window->getdtBetweenUpdates();
		if (timer > 20000.f) { timer = timer - 20000.f; }
		light.pos_.x = 40 * (cos(M_PI * timer / 10000.f));
		light.pos_.z = 40 * (sin(M_PI * timer / 10000.f));
		light.pos_.y = 40 + (20 * cos(M_PI * timer / 5000.f));

		// start GL code
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
		// draw from light's point of view
		shadowmap->bind();
        glDrawBuffer(GL_NONE);
		{
			glCullFace(GL_FRONT);
			glViewport(0, 0, shadowmap->getWidth(), shadowmap->getHeight());
			// clear to the far plane value
            //glClearColor(1.f, 1.f, 1.f, 1.f);
            //glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			lmats.pushMatrix(MatrixStack::MODELVIEW);
			lmats.pushMatrix(MatrixStack::PROJECTION);
			
			lmats.loadIdentity(MatrixStack::MODELVIEW);
			lmats.loadIdentity(MatrixStack::PROJECTION);
			light.multPovMatrix(lmats);
			
			shadowgen.use();
			lmats.initUniformLocs(shadowgen.getUniformLocation("mvMatrix"), shadowgen.getUniformLocation("pjMatrix"));
            //~ lmats.pushMatrix(MatrixStack::MODELVIEW);
			lmats.matrixToUniform(MatrixStack::MODELVIEW);
			lmats.matrixToUniform(MatrixStack::PROJECTION);
			gquad.draw();
            //~ lmats.popMatrix(MatrixStack::MODELVIEW);
			testobj.draw(shadowgen);
			
			//~ colshader.use();
			//~ mats.initUniformLocs(colshader.getUniformLocation("mvMatrix"), colshader.getUniformLocation("pjMatrix"));
			//~ mats.pushMatrix(MatrixStack::MODELVIEW);
			//~ light.multDrawMatrix(mats);
			//~ mats.matrixToUniform(MatrixStack::MODELVIEW);
			//~ mats.matrixToUniform(MatrixStack::PROJECTION);
			//~ light.mesh_.draw();
			//~ mats.popMatrix(MatrixStack::MODELVIEW);
			//~ glUseProgram(0);
			
			//lmats.popMatrix(MatrixStack::MODELVIEW);
			//lmats.popMatrix(MatrixStack::PROJECTION);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDrawBuffer(GL_BACK);
		
		// draw from camera's point of view
		{
			glCullFace(GL_BACK);
			glViewport(0, 0, window->getWidth(), window->getHeight());
			fbuf->bind();
			glClearColor(0,0,0,0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			cam.toMatrixAll(mats);

			glActiveTexture(GL_TEXTURE0);
			tex.bind();
			
			glActiveTexture(GL_TEXTURE3);
			shadowmap->bindDepthTexture();
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			
			glActiveTexture(GL_TEXTURE4);
            shadowmap->bindDepthTexture();
			smapRender.bind(4);
//            glGenerateMipmap(GL_TEXTURE_2D);

			//~ shadowmap->bindColorTexture();
			
			shadowshader.use();
			mats.initUniformLocs(shadowshader.getUniformLocation("mvMatrix"), shadowshader.getUniformLocation("pjMatrix"));
			//~ mats.pushMatrix(MatrixStack::MODELVIEW);
			mats.matrixToUniform(MatrixStack::MODELVIEW);
			mats.matrixToUniform(MatrixStack::PROJECTION);
			lmats.initUniformLocs(shadowshader.getUniformLocation("lightmv"), shadowshader.getUniformLocation("lightpj"));
			//~ glUniform1f(shadowshader.getUniformLocation("Ka"), 0.2);
			//~ glUniform1f(shadowshader.getUniformLocation("Kd"), 0.8);
			glUniform1i(shadowshader.getUniformLocation("map_Ka"), 0);
			glUniform1i(shadowshader.getUniformLocation("map_Kd"), 0);
			glUniform1i(shadowshader.getUniformLocation("shadow"), 3);
			glUniform1i(shadowshader.getUniformLocation("shadowTex"), 4);
			glUniform3fv(shadowshader.getUniformLocation("light_Pos"), 1, &light.pos_.x);
			//~ glUniform1f(shadowshader.getUniformLocation("lightNearPlane"), light.near_);
			//~ glUniform1f(shadowshader.getUniformLocation("lightFarPlane"), light.far_);
			
            //~ lmats.pushMatrix(MatrixStack::MODELVIEW);
			lmats.matrixToUniform(MatrixStack::MODELVIEW);
			lmats.matrixToUniform(MatrixStack::PROJECTION);
			gquad.draw();
            //~ lmats.popMatrix(MatrixStack::MODELVIEW);
			//~ mats.popMatrix(MatrixStack::MODELVIEW);
			
			mats.matrixToUniform(MatrixStack::MODELVIEW);
			mats.matrixToUniform(MatrixStack::PROJECTION);
			lmats.matrixToUniform(MatrixStack::MODELVIEW);
			testobj.draw(shadowshader);
			lmats.popMatrix(MatrixStack::MODELVIEW);
			lmats.popMatrix(MatrixStack::PROJECTION);
			
			colshader.use();
			mats.initUniformLocs(colshader.getUniformLocation("mvMatrix"), colshader.getUniformLocation("pjMatrix"));
			mats.pushMatrix(MatrixStack::MODELVIEW);
			light.multDrawMatrix(mats);
			mats.matrixToUniform(MatrixStack::MODELVIEW);
			mats.matrixToUniform(MatrixStack::PROJECTION);
			light.mesh_.draw();
			mats.popMatrix(MatrixStack::MODELVIEW);
			glUseProgram(0);
			glBindSampler(4, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			glActiveTexture(GL_TEXTURE0);
			fbuf->bindColorTexture();
			
			texshader.use();
			mats.loadIdentity(MatrixStack::MODELVIEW);
			mats.loadIdentity(MatrixStack::PROJECTION);
			mats.ortho(0, 1, 0, 1);
			mats.initUniformLocs(texshader.getUniformLocation("mvMatrix"), texshader.getUniformLocation("pjMatrix"));
			mats.matrixToUniform(MatrixStack::MODELVIEW);
			mats.matrixToUniform(MatrixStack::PROJECTION);
			quad.draw();
			
			// draw the shadow map
            shadowmap->bindDepthTexture();
			smapRender.bind(0);
			mats.pushMatrix(MatrixStack::MODELVIEW);
            mats.scale(0.4f / window->getAspect(), 0.4, 1.0);
			mats.matrixToUniform(MatrixStack::MODELVIEW);
			mats.matrixToUniform(MatrixStack::PROJECTION);
			quad.draw();
			mats.popMatrix(MatrixStack::MODELVIEW);
			
			glUseProgram(0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindSampler(0, 0);
		}
	
		// end GL code
		window->finishFrame();
		Sleep(1);
	}
	delete fbuf;
	return 0;
}
