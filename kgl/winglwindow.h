#ifndef WINGLWINDOW_H
#define WINGLWINDOW_H
#include "glwindow.h"
#include <windows.h>
#include <gl/wglext.h>

class WinGLWindow : public GLWindow
{
	public:
		WinGLWindow();
		virtual ~WinGLWindow();
	protected:
		void initContext();
		void initFunctions();
	
		HWND hwnd_;
};
#endif // WINGLWINDOW_H