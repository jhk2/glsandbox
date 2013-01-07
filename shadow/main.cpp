#include "../kgl/winglbase.h"
#include <stdio.h>
#include <gl/gl.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WinGLBase* window = new WinGLBase(hInstance, 800, 600);
	window->showWindow(nShowCmd);
	glClearColor(0,1.0,0,1.0);
	while(window->isActive()) {
		window->handleMessages();
		// start GL code
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// end GL code
		window->swapBuffers();
	}
	return 0;
}