#include "winglwindow.h"

WinGLWindow::WinGLWindow() : GLWindow()
{
	
}

WinGLWindow::~WinGLWindow()
{
	
}

WinGLWindow::initContext()
{
	// TODO: fix this so that it makes sense
	WNDCLASSEX wc;
	HWND hwnd;
	MSG msg;
	
	// register window class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_windowClass;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	
	unsigned int winWidth = WINDOW_WIDTH;
	unsigned int winHeight = WINDOW_HEIGHT;
	
	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	
	// create window
	hwnd = CreateWindowEx(
		WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		g_windowClass,
		"Title Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT, winWidth, winHeight,
		NULL, NULL, hInstance, NULL);
	
	if(hwnd == NULL)
	{
		MessageBox(NULL, "Window Creation Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	
	HDC hdc;
	if(!(hdc = GetDC(hwnd)))
		printf("GetDC failed\n");
	
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
		printf("ChoosePixelFormat failed\n");
	//*/
	
	// assign pixel format to device context
	if(!(SetPixelFormat(hdc, iPixelFormat, &pfd)))
		printf("SetPixelFormat failed\n");
	
	// create  opengl context
	HGLRC context;
	if(!(context = wglCreateContext(hdc)))
		printf("wglCreateContext failed\n");
	if(!(wglMakeCurrent(hdc, context)))
		printf("wglMakeCurrent failed\n");
	
	// Now we want an updated pixel format and context
	//*
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if(!wglChoosePixelFormatARB)
	{
		printf("wgl choose pixel format not supported?\n");
	}
	
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	if(!wglCreateContextAttribsARB)
		printf("wglCreateContextAttribsARB undefined\n");
	
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
		printf("we have a valid format\n");
	} else {
		printf("wglchoosepixel format didn't find a valid format\n");
	}
	// if we found a valid format, it is stored in ePixelFormat
	// delete old rendering context
	int delc = wglDeleteContext(context);
	if (!delc)
		printf("failed to delete old context\n");
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
		CW_USEDEFAULT, winWidth, winHeight,
		NULL, NULL, hInstance, NULL);
	// get the new device context
	if(!(hdc = GetDC(hwnd)))
		printf("second GetDC failed\n");
	// set the pixel format the the extended one we got earlier
	if (!SetPixelFormat(hdc, ePixelFormat, &pfd))
	{
		// failed to set pixel format
		printf("failed to set extended pixel format\n");
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
		printf("second wglCreateContext failed\n");
	if(!(wglMakeCurrent(hdc, context)))
		printf("second wglMakeCurrent failed\n");
	//*/
	
	ShowWindow(hwnd, nCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	UpdateWindow(hwnd);
	
	// set initial mouse position
	POINT p;
	if(GetCursorPos(&p)) {
		if (ScreenToClient(hwnd, &p)) {
			g_mouse_x = p.x;
			g_mouse_y = p.y;
		} else {
			printf("ScreenToClient failed\n");
		}
	} else {
		printf("GetCursorPos failed\n");
	}
	
	hwnd_ = hwnd;
}

WinGLWindow::initFunctions()
{
	
}