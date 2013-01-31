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
		void resize(unsigned int width, unsigned int height);
		void update();
		long double currentMillis();
		float getdtBetweenUpdates();
		void showWindow(int nCmdShow);
		void close();
		
		void loadExtensions();
	
		long getExitCode() { return exitCode_; }
		
		// IO stuff
		bool isKeyDown(const unsigned int code);
		bool isKeyPress(const unsigned int code);
		bool isKeyRelease(const unsigned int code);
		bool isMouseDown(const MOUSE_BUTTON code);
		bool isMousePress(const MOUSE_BUTTON code);
		bool isMouseRelease(const MOUSE_BUTTON code);
		
		
		// mouse position
		int2 getMousePixelPos();
		int2 getMousePixel_dxdy();
		fl2 getMouseNormPos();
		fl2 getMouseNorm_dxdy();
		
		// mouse cursor hide/show
		void showCursor();
		void hideCursor();
		void holdCursor(const bool setting);
		
		// adding additional message handlers
		typedef long (* MessageHandler)(WinGLBase &, HWND, WPARAM, LPARAM);
		MessageHandler WinGLBase::addMessageHandler(long message, MessageHandler handler);
		
	protected:
		void handleMessages();
		void swapBuffers();
		void swapIODeviceBuffers();
		bool initContext();
		bool initFunctions();
	
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
		// keyboard message handlers
		static long OnKeyDown(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam);
		static long OnKeyUp(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam);
		// mouse button handlers
		static long OnMouseDownL(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam);
		static long OnMouseUpL(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam);
		static long OnMouseDownM(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam);
		static long OnMouseUpM(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam);
		static long OnMouseDownR(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam);
		static long OnMouseUpR(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam);
	
		// mouse and keyboard related stuff
		unsigned int frontKeyBuffer_;
		bool keys_[2][256];
		bool mouse_buttons_[2][3];
		int2 mouse_pos_;
		int2 mouse_dxdy_;
		int2 hold_pos_;
		bool holdcursor_;
		
		long double current_millis_;
		long double dt_;
		
};
// enum for virtual key codes
enum WIN_KEYS {
	// alphabet
	KEY_A=0x41,KEY_B=0x42,KEY_C=0x43,KEY_D=0x44,KEY_E=0x45,KEY_F=0x46,KEY_G=0x47,KEY_H=0x48,
	KEY_I=0x49,KEY_J=0x4A,KEY_K=0x4B,KEY_L=0x4C,KEY_M=0x4D,KEY_N=0x4E,KEY_O=0x4F,KEY_P=0x50,
	KEY_Q=0x51,KEY_R=0x52,KEY_S=0x53,KEY_T=0x54,KEY_U=0x55,KEY_V=0x56,KEY_W=0x57,KEY_X=0x58,
	KEY_Y=0x59,KEY_Z=0x5A,
	// todo: add other keys
	KEY_ESC=0x1B,
};
#endif // WINGLBASE_H