#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>

void (__stdcall*glUseProgram) (unsigned int);

void renderGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glVertex2f(0, -1);
	glVertex2f(0, 1);
	glVertex2f(1, 1);
	glVertex2f(1, -1);
	glEnd();
	*/
	glUseProgram(0);
}

void initGL()
{
	glClearColor(0, 0, 0, 0);
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glUseProgram = (void(__stdcall*)(unsigned int)) wglGetProcAddress("glUseProgram");
}

void resizeGL(unsigned int width, unsigned int height)
{
	glViewport(0, 0, width, height);
}

const char g_windowClass[] = "glSandboxWindowClass";

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// console for debugging
	//AllocConsole();
	//freopen("CONOUT$","wb",stdout);

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
	
	unsigned int winWidth = 640;
	unsigned int winHeight = 480;
	
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
	HDC hdc;
	if(!(hdc = GetDC(hwnd)))
		printf("GetDC failed\n");
	
	if(!(iPixelFormat = ChoosePixelFormat(hdc, &pfd)))
		printf("ChoosePixelFormat failed\n");
	
	// assign pixel format to device context
	if(!(SetPixelFormat(hdc, iPixelFormat, &pfd)))
		printf("SetPixelFormat failed\n");
	
	// create  opengl context
	HGLRC context;
	if(!(context = wglCreateContext(hdc)))
		printf("wglCreateContext failed\n");
	if(!(wglMakeCurrent(hdc, context)))
		printf("wglMakeCurrent failed\n");
	
	ShowWindow(hwnd, nCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	resizeGL(winWidth, winHeight);
	UpdateWindow(hwnd);
	
	initGL();
	printf("finished init, starting main loop\n");
	// main loop
	bool running = true;
	while(running)
	{
		while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if(GetMessage(&msg, NULL, 0, 0) > 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				running = false;
				break;
			}
		}
		
		// draw stuff
		renderGL();
		SwapBuffers(hdc);
		
	}
	
	//FreeConsole();
	// delete the rendering context
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(context);
	return msg.wParam;
}