#include "winglbase.h"

WinGLBase::WinGLBase(HINSTANCE hInstance, unsigned int width, unsigned int height) : GLBase(width, height)
{
	hInstance_ = hInstance;
	addMessageHandler(WM_CLOSE, OnClose);
	addMessageHandler(WM_DESTROY, OnDestroy);
	initContext();
}

WinGLBase::~WinGLBase()
{
	
}

void WinGLBase::swapBuffers()
{
	SwapBuffers(hdc_);
}

void WinGLBase::update()
{
	
}

void WinGLBase::throwError(const std::string &message)
{
	MessageBox(NULL, message.c_str(), "Error", MB_ICONEXCLAMATION | MB_OK);
}

void WinGLBase::handleMessages()
{
	static MSG msg;
	if(!hwnd_)
		throwError("handle messages called on null hwnd");
	while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		int temp = GetMessage(&msg, NULL, 0, 0);
		if(temp > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			//throwError("getmessage returned nonpositive");  not an error if it's a quit
			running_ = false;
			exitCode_ = (long) msg.lParam;
			break;
		}
	}
}

WinGLBase::MessageHandler WinGLBase::addMessageHandler(long message, MessageHandler handler)
{
	// insert the message handler into the map and return the old one that was replaced, if it exists
	MessageHandler m = NULL;
	MessageIterator it = messagemap_.find(message);
	if(it != messagemap_.end())
		m = it->second;
	messagemap_.insert(std::pair<long, MessageHandler>(message, handler));
	return m;
}

bool WinGLBase::getMessageHandler(long message, WinGLBase::MessageIterator &it)
{
	// look in the message map for the handler
	WinGLBase::MessageIterator findit = messagemap_.find(message);
	if(findit == messagemap_.end())
		return false;
	it = findit;
	return true;
}

LRESULT CALLBACK WinGLBase::RouteMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	printf("routemessage called\n");
	WinGLBase *wnd = 0;
	if(message == WM_NCCREATE) {
		// when the window is created, we want to save its unique ID to GWL_USERDATA so we can retreive this particular window
		// in a static context later when more messages come in
	
		// get the individual window instance from Windows
		wnd = reinterpret_cast<WinGLBase *>(((LPCREATESTRUCT) lparam)->lpCreateParams);
		SetWindowLong(hwnd, GWL_USERDATA, reinterpret_cast<long>(wnd));
		// save the hwnd
		printf("got nccreate, set hwnd to %x\n", hwnd);
		wnd->setHWND(hwnd);
	} else {
		// get the individual window from GWL_USERDATA
		wnd = reinterpret_cast<WinGLBase *>(GetWindowLong(hwnd, GWL_USERDATA));
	}
	if(wnd) {
		WinGLBase::MessageIterator it;
		// get the message handler from our window's message map
		bool found = wnd->getMessageHandler(message, it);
		if(found) {
			// call the function that the message map points to
			return (it->second)((*wnd), hwnd, wparam, lparam);
		}
	}
	// otherwise return the default window proc
	return DefWindowProc(hwnd, message, wparam, lparam);
}

const char g_windowClass[] = "WinGLBaseWindowClass";

bool WinGLBase::initContext()
{
	// TODO: fix this so that it makes sense
	WNDCLASSEX wc;
	HWND hwnd;
	MSG msg;
	
	// register window class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = RouteMessage;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance_;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_windowClass;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	
	if(!RegisterClassEx(&wc))
	{
		throwError("Window Registration Failed");
		return false;
	}
	
	// create window
	hwnd = CreateWindowEx(
		WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		g_windowClass,
		"Title Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT, width_, height_,
		NULL, NULL, hInstance_, this);
	
	if(hwnd == NULL)
	{
		throwError("Window Creation Failed");
		printf("getlasterror value %x\n", GetLastError());
		return false;
	}
	
	HDC hdc;
	if(!(hdc = GetDC(hwnd)))
	{
		throwError("GetDC failed");
		return false;
	}
	
	//* old set pixel format
	// set pixel format
	PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // flags
		PFD_TYPE_RGBA, // rgba framebuffer
		32, // 32 bit color depth
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		24, 8, // 24 bit depth, 8 bit stencil
		0, // # of aux buffers
		PFD_MAIN_PLANE,
		0, 0, 0, 0
	};
	
	// get available matching pixel format
	int iPixelFormat;

	if(!(iPixelFormat = ChoosePixelFormat(hdc, &pfd)))
	{
		throwError("ChoosePixelFormat failed");
		return false;
	}
	//*/
	
	// assign pixel format to device context
	if(!(SetPixelFormat(hdc, iPixelFormat, &pfd)))
	{
		throwError("SetPixelFormat failed");
		return false;
	}
	
	// create  opengl context
	HGLRC context;
	if(!(context = wglCreateContext(hdc)))
	{
		throwError("wglCreateContext failed");
		return false;
	}
	if(!(wglMakeCurrent(hdc, context)))
	{
		throwError("wglMakeCurrent failed");
		return false;
	}
	
	// Now we want an updated pixel format and context
	//*
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if(!wglChoosePixelFormatARB)
	{
		throwError("wgl choose pixel format not supported?");
		return false;
	}
	
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	if(!wglCreateContextAttribsARB)
	{
		throwError("wglCreateContextAttribsARB undefined\n");
		return false;
	}
	
	// using wglchoosepixelformat
	const int attribList[] = 
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		0,
	};
	
	int ePixelFormat;
	unsigned int numFormats;
	int valid = wglChoosePixelFormatARB(hdc, attribList, NULL, 1, &ePixelFormat, &numFormats);
	
	if (valid && numFormats >= 1)
	{
		// we have a valid format
		//throwError("we have a valid format"); not an error
	} else {
		throwError("wglchoosepixel format didn't find a valid format");
		return false;
	}
	// if we found a valid format, it is stored in ePixelFormat
	// delete old rendering context
	int delc = wglDeleteContext(context);
	if (!delc)
	{
		throwError("failed to delete old context");
		return false;
	}
	// release device context
	ReleaseDC(hwnd, hdc);
	// destroy the window
	DestroyWindow(hwnd);
	
	while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		// get rid of the first destroy message so it doesn't screw up later
		int temp = GetMessage(&msg, NULL, 0, 0);
		if (temp != 0) {
			printf("whoops, something other than destroy was in message queue after destroywindow: (%i)\n", temp);
		} else {
			printf("disposed of the first destory message\n");
		}
	}
	
	// now, make it all again
	hwnd = CreateWindowEx(
		WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		g_windowClass,
		"Title Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT, width_, height_,
		NULL, NULL, hInstance_, this);
	// get the new device context
	if(!(hdc = GetDC(hwnd)))
	{
		throwError("second GetDC failed");
		return false;
	}
	// set the pixel format the the extended one we got earlier
	if (!SetPixelFormat(hdc, ePixelFormat, &pfd))
	{
		// failed to set pixel format
		throwError("failed to set extended pixel format");
		return false;
	}
	
	// create extended opengl rendering context
	int contextAttribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 2,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		//WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};
	
	
	if(!(context = wglCreateContextAttribsARB(hdc, NULL, contextAttribs)))
	{
		throwError("second wglCreateContext failed");
		return false;
	}
	if(!(wglMakeCurrent(hdc, context)))
	{
		throwError("second wglMakeCurrent failed");
		return false;
	}
	//*/
	
	hwnd_ = hwnd;
	hdc_ = hdc;
	running_ = true;
	return true;
}

void WinGLBase::showWindow(int nCmdShow)
{
	ShowWindow(hwnd_, nCmdShow);
	SetForegroundWindow(hwnd_);
	SetFocus(hwnd_);
	UpdateWindow(hwnd_);
	
	// set initial mouse position
	POINT p;
	if(GetCursorPos(&p)) {
		if (ScreenToClient(hwnd_, &p)) {
			//g_mouse_x = p.x;
			//g_mouse_y = p.y;
		} else {
			throwError("ScreenToClient failed");
		}
	} else {
		throwError("GetCursorPos failed");
	}
}

bool WinGLBase::initFunctions()
{
	return false;
}

long long WinGLBase::currentMillis()
{
	LARGE_INTEGER freq;
	BOOL use_qpc = QueryPerformanceFrequency(&freq);
	if (use_qpc) {
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return (1000LL * now.QuadPart) / freq.QuadPart;
	} else {
		return GetTickCount();
	}
}

void WinGLBase::setHWND(HWND hwnd)
{
	hwnd_ = hwnd;
}

// default message handler for closing
long WinGLBase::OnClose(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	DestroyWindow(hwnd);
	return 0;
}

// default message handler for destroy
long WinGLBase::OnDestroy(WinGLBase &wnd, HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	PostQuitMessage(0);
	return 0;
}