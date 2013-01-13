#include "../kgl/winglbase.h"
#include "../kgl/glextensionfuncs.h"
#include "../kgl/matrixstack.h"
#include "../kgl/shader.h"
#include <stdio.h>

long OnResize(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	//~ printf("resize\n"); fflush(stdout);
	// lparam loword has width, hiword has height
	wnd.resize(LOWORD(lparam), HIWORD(lparam));
	// change any camera related stuff here
	
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WinGLBase* window = new WinGLBase(hInstance, 800, 600);
	window->addMessageHandler(WM_SIZE, OnResize);
	window->showWindow(nShowCmd);
	MatrixStack mats;
	Shader shader ("shader.glsl", Shader::VERTEX_SHADER | Shader::FRAGMENT_SHADER);
	while(window->isActive()) {
		// always call at the beginning of loop iteration (should we combine with isActive and finishFrame?)
		window->update();
		glClearColor(0,0,0,0);
		if(window->isKeyPress(KEY_ESC)) {
			window->close();
			break;
		}
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
		// start GL code
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// end GL code
		window->finishFrame();
	}
	return 0;
}