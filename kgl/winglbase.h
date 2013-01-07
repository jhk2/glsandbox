#ifndef WINGLBASE_H
#define WINGLBASE_H
#include "glbase.h"
#include <windows.h>
#include <gl/gl.h>
#include <gl/wglext.h>
#include <string>
#include <map>

/*
*	Overall design based on Win32 Wrapper class at
*	http://www.gamedev.net/page/resources/_/technical/general-programming/creating-a-win32-window-wrapper-class-r1810
*/

class WinGLBase : public GLBase
{
	public:
		WinGLBase(HINSTANCE hInstance, unsigned int width, unsigned int height);
		virtual ~WinGLBase();
		void swapBuffers();
		void update();
		long long currentMillis();
		void showWindow(int nCmdShow);
		void handleMessages();
	
		long getExitCode() { return exitCode_; }
	
	protected:
		bool initContext();
		bool initFunctions();
	
		typedef long (* MessageHandler)(WinGLBase &, HWND, WPARAM, LPARAM);
		// adding additional message handlers
		MessageHandler WinGLBase::addMessageHandler(long message, MessageHandler handler);
	
	private:
		HINSTANCE hInstance_;
		long exitCode_;
	
		// static windows message handler function
		static LRESULT CALLBACK RouteMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		void setHWND(HWND hwnd);
		void throwError(const std::string &message);
		HWND hwnd_;
		HDC hdc_;
	
		// map for storing message handlers
		typedef std::map<long, MessageHandler> MessageMap;
		typedef MessageMap::iterator MessageIterator;
		MessageMap messagemap_;
		bool getMessageHandler(long message, MessageIterator &it);
	 
		// some default message handlers
		static long OnClose(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam);
		static long OnDestroy(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam);
};
#endif // WINGLBASE_H