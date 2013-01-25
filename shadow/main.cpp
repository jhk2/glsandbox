#include "../kgl/winglbase.h"
#include "../kgl/glextensionfuncs.h"
#include "../kgl/matrixstack.h"
#include "../kgl/shader.h"
#include "../kgl/mesh.h"
#include <stdio.h>
#include "../kgl/debug.h"

MatrixStack mats;

long OnResize(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	//~ printf("resize\n"); fflush(stdout);
	// lparam loword has width, hiword has height
	wnd.resize(LOWORD(lparam), HIWORD(lparam));
	// change any camera related stuff here
	mats.loadIdentity(MatrixStack::MODELVIEW);
	mats.loadIdentity(MatrixStack::PROJECTION);
	mats.ortho(0, 1, 1, 0);
	
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WinGLBase* window = new WinGLBase(hInstance, 800, 600);
	window->addMessageHandler(WM_SIZE, OnResize);
	window->showWindow(nShowCmd);
	printf("make shader\n"); fflush(stdout);
	Shader shader ("shader.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
	//~ glBindFragDataLocation(shader.getProgramID(), 0, "out_Color");
	// get uniform locations
	GLint mvloc = shader.getUniformLocation("mvMatrix");
	GLint pjloc = shader.getUniformLocation("pjMatrix");
	checkGLError("getuniformlocation");
	
	printf("init matrix uniform locs at %i, %i\n", mvloc, pjloc);
	mats.initUniformLocs(mvloc, pjloc);
	
	printf("init quad data\n"); fflush(stdout);
	// let's make a simple quad for a mesh
	//~ std::vector<std::pair<unsigned int, AttributeInfo*>> attribInfo;
	//~ printf("attribinfo vector created\n"); fflush(stdout);
	//~ attribInfo.push_back(std::pair<unsigned int, AttributeInfo*>(0, new AttributeInfoSpec<GLfloat>(GL_FLOAT, 3)));
	printf("making vert vector\n"); fflush(stdout);
	std::vector<fl3> verts;
	verts.push_back(fl3(0, 0, 0));
	verts.push_back(fl3(0, 1, 0));
	verts.push_back(fl3(1, 1, 0));
	verts.push_back(fl3(1, 0, 0));
	printf("verts vector created\n"); fflush(stdout);
	//~ VertexBufferSpec<fl3> vbuf (&verts);
	std::vector<unsigned int> inds;
	inds.push_back(0); inds.push_back(1); inds.push_back(2); inds.push_back(3);
	printf("make mesh from quad data\n"); fflush(stdout);
	//~ Mesh<fl3> quad (attribInfo, vbuf, inds);
	//~ printf("mesh created\n"); fflush(stdout);
	
	Mesh<fl3, GLuint> quad;
	quad.addAttrib(0, AttributeInfoSpec<GLfloat>(3)).addVerts(verts).addInds(inds).finalize();
	//~ quad.addAttrib(0, AttributeInfoSpec<GLfloat>(3));
	//~ quad.addVerts(verts);
	//~ quad.addInds(inds);
	//~ quad.finalize();
	
	while(window->isActive()) {
		// always call at the beginning of loop iteration (should we combine with isActive and finishFrame?)
		window->update();
		glClearColor(0,0,0,0);
		
		if(window->isKeyPress(KEY_ESC)) {
			window->close();
			break;
		}
		
		/*
		if(window->isMouseDown(MOUSE_LEFT)) {
			glClearColor(1.0,0,0,1.0);
		} else if(window->isMouseDown(MOUSE_RIGHT)) {
			glClearColor(0,1.0,0,1.0);
		} else if(window->isKeyDown(KEY_W)) {
			glClearColor(0,0,1.0,1.0);
		}
		if(window->isMousePress(MOUSE_RIGHT)) {
			window->hideCursor();
			window->holdCursor(true);
		} else if(window->isMouseRelease(MOUSE_RIGHT)) {
			window->showCursor();
			window->holdCursor(false);
		}
		*/
		// start GL code
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		shader.use();
		mats.matrixToUniform(MatrixStack::MODELVIEW);
		mats.matrixToUniform(MatrixStack::PROJECTION);
		quad.draw();
		glUseProgram(0);
		
		// end GL code
		window->finishFrame();
	}
	return 0;
}