#include "../kgl/winglbase.h"
#include <stdio.h>
#include <gl/gl.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WinGLBase* window = new WinGLBase(hInstance, 800, 600);
	window->showWindow(nShowCmd);
	while(window->isActive()) {
		glClearColor(0,0,0,0);
		window->update();
		if(window->isMouseDown(GLBase::MOUSE_LEFT)) {
			glClearColor(1.0,0,0,1.0);
		} else if(window->isMouseDown(GLBase::MOUSE_RIGHT)) {
			glClearColor(0,1.0,0,1.0);
		} else if(window->isKeyDown(WinGLBase::KEY_W)) {
			glClearColor(0,0,1.0,1.0);
		}
		if(window->isMousePress(GLBase::MOUSE_RIGHT)) {
			window->hideCursor();
			window->holdCursor(true);
		} else if(window->isMouseRelease(GLBase::MOUSE_RIGHT)) {
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