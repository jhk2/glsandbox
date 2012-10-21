#include <windows.h>
#include <gl/gl.h>
#include <gl/glext.h>
#include <gl/wglext.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define SHADER_SRC "shader.glsl"

HWND g_hwnd;
bool g_running = true;
// front and back buffer keys
unsigned int frontKeyBuffer = 0;
bool keys[2][256];
bool mouse[2][3];
int g_mouse_x, g_mouse_y;
int g_hold_x, g_hold_y;

// functions
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLDETACHSHADERPROC glDetachShader;
PFNGLDELETESHADERPROC glDeleteShader;
//PFNGLDRAWELEMENTSPROC glDrawElements;

//unsigned int vao;
unsigned int vbuf;
unsigned int nbuf;
unsigned int ibuf;
unsigned int ibufcount;

// structs
struct fl3 {
	union {
		struct {
			float x, y, z;
		};
		float xyz[3];
	};
	fl3() : x(0), y(0), z(0) {};
	fl3(float nx, float ny, float nz) : x(nx), y(ny), z(nz) {};
	void fl3::normalize() 
	{
		float magnitude = sqrt(x*x + y*y + z*z);
		if (magnitude == 0) 
			return;
		x /= magnitude;
		y /= magnitude;
		z /= magnitude;
	};
	fl3 fl3::operator*(const float &scalar)
	{
		return fl3(x*scalar, y*scalar, z*scalar);
	};
	fl3 fl3::operator+(const fl3 &other)
	{
		return fl3(x+other.x, y+other.y, z+other.z);
	}
};

struct fl2 {
	union {
		struct {
			float x, y;
		};
		struct {
			float s, t;
		};
		float xy[2];
		float st[2];
	};
};

// models
std::vector<fl3> verts;
std::vector<fl3> norms;
std::vector<unsigned int> inds;

// camera stuff
fl2 g_cam_rot;
fl3 g_cam_pos;

// shader
struct Shader {
	GLenum id;
	std::vector<GLuint> progs;
};
Shader g_shader;
// modification time for shader
ULONGLONG g_shader_modified;

// time calculation function
long long currentMillis() {
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

void renderGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/* draw immediate scene
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.5f, 0, -6);
	glBegin(GL_TRIANGLES);
	glVertex2f(0, 1);
	glVertex2f(-1, -1);
	glVertex2f(1, -1);
	glEnd();
	glTranslatef(3, 0, 0);
	glBegin(GL_QUADS);
	glVertex2f(-1, 1);
	glVertex2f(1, 1);
	glVertex2f(1, -1);
	glVertex2f(-1, -1);
	glEnd();
	// end immediate scene */
	
	glUseProgram(g_shader.id);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(g_cam_rot.x, 1, 0, 0);
	glRotatef(g_cam_rot.y, 0, 1, 0);
	glTranslatef(g_cam_pos.x, g_cam_pos.y, g_cam_pos.z);
	glScalef(0.02, 0.02, 0.02);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuf);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawElements(GL_TRIANGLES, ibufcount, GL_UNSIGNED_INT, 0);
	glUseProgram(0);
}

void printShaderLog(GLuint id)
{
	int infoLogLength = 0;
	char infoLog[1024];
	glGetShaderInfoLog(id, 1024, &infoLogLength, infoLog);
	if (infoLogLength > 0)
		printf("Shader log:\n%s", infoLog);
}

void loadShaderFromSource(GLenum type, std::string sourcefile)
{
	std::stringstream ss;
	std::ifstream file(sourcefile.c_str());
	std::string line;
	if (file.is_open() && file.good())
	{
		getline(file, line);
		// skip past whitespace at the beginning
		while (file.good() && (strcmp(line.c_str(), "\n")==0 || strcmp(line.c_str(), "")==0))
		{
			getline(file, line);
		}
		// #define the shader type
		switch (type)
		{
			case GL_FRAGMENT_SHADER:
				ss << "#define _FRAGMENT_" << std::endl;
				break;
			case GL_VERTEX_SHADER:
				ss << "#define _VERTEX_" << std::endl;
				break;
			case GL_GEOMETRY_SHADER:
				ss << "#define _GEOMETRY_" << std::endl;
				break;
			case GL_TESS_CONTROL_SHADER:
				ss << "#define _TESSCONTROL_" << std::endl;
				break;
			case GL_TESS_EVALUATION_SHADER:
				ss << "#define _TESSEVAL_" << std::endl;
				break;
			default:
				break;
		}
		ss << line << std::endl;
		while (file.good())
		{
			getline(file, line);
			ss << line << std::endl;
		}
		file.close();
		// load into gl
		std::string str = ss.str();
		//printf("shader source is:\n%s\n", str);
		const int length = str.length();
		const char *text = str.c_str();
		GLuint id = glCreateShader(type);
		glShaderSource(id, 1, (const char **)&text, &length);
		glCompileShader(id);
		printShaderLog(id);
		glAttachShader(g_shader.id, id);
		glDeleteShader(id);
		g_shader.progs.push_back(id);
	}
}

void refreshShaderProgram(std::string sourcefile)
{
	loadShaderFromSource(GL_VERTEX_SHADER, sourcefile);
	loadShaderFromSource(GL_FRAGMENT_SHADER, sourcefile);
	glLinkProgram(g_shader.id);
}

void loadShaderProgram(std::string sourcefile)
{
	//printf("loading shader from %s\n", sourcefile.c_str());
	g_shader.id = glCreateProgram();
	refreshShaderProgram(sourcefile);
	
	FILETIME create, access, write;
	HANDLE fhandle = CreateFile(SHADER_SRC, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	GetFileTime(fhandle, &create, &access, &write);
	ULONGLONG time = (((ULONGLONG) write.dwHighDateTime) << 32) + write.dwLowDateTime;
	g_shader_modified = time;
	CloseHandle(fhandle);
}
void loadFunctions()
{
	//glUseProgram = (void(__stdcall*)(unsigned int)) wglGetProcAddress("glUseProgram");
	glBindBuffer = (PFNGLBINDBUFFERPROC) wglGetProcAddress("glBindBuffer");
	glGenBuffers = (PFNGLGENBUFFERSPROC) wglGetProcAddress("glGenBuffers");
	glBufferData = (PFNGLBUFFERDATAPROC) wglGetProcAddress("glBufferData");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glEnableVertexAttribArray");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) wglGetProcAddress("glVertexAttribPointer");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) wglGetProcAddress("glGetShaderInfoLog");
	glCreateShader = (PFNGLCREATESHADERPROC) wglGetProcAddress("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC) wglGetProcAddress("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC) wglGetProcAddress("glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC) wglGetProcAddress("glCreateProgram");
	glLinkProgram = (PFNGLLINKPROGRAMPROC) wglGetProcAddress("glLinkProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC) wglGetProcAddress("glAttachShader");
	glUseProgram = (PFNGLUSEPROGRAMPROC) wglGetProcAddress("glUseProgram");
	glDetachShader = (PFNGLDETACHSHADERPROC) wglGetProcAddress("glDetachShader");
	glDeleteShader = (PFNGLDELETESHADERPROC) wglGetProcAddress("glDeleteShader");
	//glDrawElements = (PFNGLDRAWELEMENTSPROC) wglGetProcAddress("glDrawElements");
}

void loadResources()
{	
	printf("loading model\n");
	std::ifstream myfile ("lego.obj");
	std::string line;
	if (myfile.is_open()) {
		printf("opened file\n");
		// read in the obj file
		while (myfile.good())
		{
			std::getline(myfile, line);
			// line has the contents
			std::string sub = line.substr(0, 2);
			if (!sub.compare("v ")) {
				// it's a vertex
				fl3 v;
				sscanf(line.c_str(), "%*s %f %f %f", &v.x, &v.y, &v.z);
				verts.push_back(v);
				//printf("added vert: %f, %f, %f\n", v.x, v.y, v.z);
			} else if (!sub.compare("vn")) {
				// normal
				fl3 n;
				sscanf(line.c_str(), "%*s %f %f %f", &n.x, &n.y, &n.z);
				norms.push_back(n);
				//printf("added norm: %f, %f, %f\n", n.x, n.y, n.z);
			} else if (!sub.compare("f ")) {
				// it's a face
				unsigned int q[3];
				sscanf(line.c_str(), "%*s %u//%*u %u//%*u %u//%*u", &q[0], &q[1], &q[2]);
				//sscanf(line.c_str(), "%*s %u %u %u", &q[0], &q[1], &q[2]);
				//printf("adding indices: %u, %u, %u\n", q[0], q[1], q[2]);
				// obj is 1 indexed, so subtract 1
				for(int i = 0; i < 3; i++)
					inds.push_back(q[i]-1);
			} else {
				
			}
		}
	} else {
		printf("load model failed\n");
	}
	myfile.close();
	printf("done model reading\n");
	
	ibufcount = inds.size();
	printf("read %i indices\n", ibufcount);
	printf("read %i verts\n", verts.size());
	printf("read %i norms\n", norms.size());
	
	// we should have all of our normals, verts, and indices
	// make a gl buffer for them
	
	//glGenVertexArrays(1, &vao);
	//glBindVertexArray(vao);
	
	glGenBuffers(1, &vbuf);
	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fl3)*verts.size(), &verts[0], GL_STATIC_DRAW);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(0);
	printf("sizeof verts: %i\n", sizeof(fl3)*verts.size());
	
	glGenBuffers(1, &nbuf);
	glBindBuffer(GL_ARRAY_BUFFER, nbuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fl3)*norms.size(), &norms[0], GL_STATIC_DRAW);
	printf("sizeof norms: %i\n", sizeof(fl3)*norms.size());
	
	glGenBuffers(1, &ibuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*inds.size(), &inds[0], GL_STATIC_DRAW);
	printf("sizeof inds: %i\n", sizeof(unsigned int)*inds.size());
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void setPerspective(double fovy, double aspect, double zNear, double zFar)
{
	double xmin, xmax, ymin, ymax;
	ymax = zNear * tan(fovy * M_PI / 360.0);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;
	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

void resizeGL(unsigned int width, unsigned int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setPerspective(45, (float)width/height, 0.1, 100.0);
}

void initGL(unsigned int width, unsigned int height)
{
	glClearColor(0, 0, 0, 0);
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);
	resizeGL(width, height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	loadFunctions();
	loadResources();
	loadShaderProgram(SHADER_SRC);
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
	//glFrontFace(GL_CW);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	// set up default camera
	g_cam_pos.x = 0;
	g_cam_pos.y = 0;
	g_cam_pos.z = -10;
	g_cam_rot.x = 0;
	g_cam_rot.y = 0;
}

bool isKeyDown(unsigned int code)
{
	return keys[frontKeyBuffer][code];
}

bool isKeyPress(unsigned int code)
{
	return keys[frontKeyBuffer][code] && !keys[1-frontKeyBuffer][code];
}

bool isKeyRelease(unsigned int code)
{
	return !keys[frontKeyBuffer][code] && keys[1-frontKeyBuffer][code];
}

float degToRad(const float degrees)
{
	return M_PI * (degrees / 180);
}

#define MOVESCALE 0.02f

void handleKeys(float dt)
{
	/*
	if(isKeyPress(0x31))
		glColor3f(1, 0, 0);
	if(isKeyPress(0x32))
		glColor3f(0, 1, 0);
	if(isKeyRelease(0x33))
		glColor3f(0, 0, 1);
	*/
	fl3 tomove;
	if(isKeyDown(0x57)) {
		// w key
		tomove.z -= cos(degToRad(g_cam_rot.y));
		tomove.x += sin(degToRad(g_cam_rot.y));
		tomove.y -= sin(degToRad(g_cam_rot.x));
	}
	if(isKeyDown(0x53)) {
		// s key
		tomove.z += cos(degToRad(g_cam_rot.y));
		tomove.x -= sin(degToRad(g_cam_rot.y));
		tomove.y += sin(degToRad(g_cam_rot.x));
	}
	if(isKeyDown(0x41)) {
		// a key
		tomove.z -= sin(degToRad(g_cam_rot.y));
		tomove.x -= cos(degToRad(g_cam_rot.y));
	}
	if(isKeyDown(0x44)) {
		// d key
		tomove.z += sin(degToRad(g_cam_rot.y));
		tomove.x += cos(degToRad(g_cam_rot.y));
	}
	if(isKeyDown(VK_SPACE)) {
		tomove.y += 1;
	}
	if(isKeyDown(VK_CONTROL)) {
		tomove.y -= 1;
	}
	tomove.normalize();
	tomove = tomove * MOVESCALE * dt;
	//printf("tomove is %g, %g, %g\n", tomove.x, tomove.y, tomove.z);
	//printf("dt is %g\n", dt);
	g_cam_pos = g_cam_pos + tomove;
}

void swapKeyBuffers()
{
	frontKeyBuffer = 1 - frontKeyBuffer;
	memcpy(&keys[frontKeyBuffer][0], &keys[1-frontKeyBuffer][0], sizeof(keys[0]));
	memcpy(&mouse[frontKeyBuffer][0], &mouse[1-frontKeyBuffer][0], sizeof(mouse[0]));
}

bool isMouseDown(const unsigned int code)
{
	return mouse[frontKeyBuffer][code];
}

bool isMousePress(const unsigned int code)
{
	return mouse[frontKeyBuffer][code] && !mouse[1-frontKeyBuffer][code];
}

bool isMouseRelease(const unsigned int code)
{
	return !mouse[frontKeyBuffer][code] && mouse[1-frontKeyBuffer][code];
}

#define MOUSESCALE 0.1f

void onMouseMove(int nx, int ny)
{
	int dx = nx - g_mouse_x;
	int dy = ny - g_mouse_y;
	
	// for now, only if right click is down
	if (isMouseDown(2)) {
		printf("old x is %i, old y is %i\n", g_mouse_x, g_mouse_y);
		printf("new x is %i, new y is %i\n", nx, ny);
		printf("dx is %i, dy is %i\n", dx, dy);
		g_cam_rot.y += MOUSESCALE * dx;
		g_cam_rot.x += MOUSESCALE * dy;
		// limit up/down rotation to -90 to +90 degrees
		g_cam_rot.x = min(90, max(-90, g_cam_rot.x));
		// limit left/right rotation to 0 -360 to 360 to prevent overflow
		if (g_cam_rot.y > 360) {
			g_cam_rot.y = 360 - g_cam_rot.y;
		} else if (g_cam_rot.y < -360) {
			g_cam_rot.y = 360 + g_cam_rot.y;
		}
	}	
	
	g_mouse_x = nx;
	g_mouse_y = ny;
}

void handleMouse(float dt)
{
	// hide cursor if rmb is down
	if (isMousePress(2)) {
		ShowCursor(false);
	} else if (isMouseRelease(2)) {
		ShowCursor(true);
	}
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(g_hwnd, &pt);
	onMouseMove(pt.x, pt.y);
	if (isMouseDown(2)) {
		if (isMousePress(2)) {
			g_hold_x = pt.x;
			g_hold_y = pt.y;
		}
		// recenter the cursor
		POINT pt;
		pt.x = g_hold_x;
		pt.y = g_hold_y;
		ClientToScreen(g_hwnd, &pt);
		SetCursorPos(pt.x, pt.y);
		g_mouse_x = g_hold_x;
		g_mouse_y = g_hold_y;
	}
}

// global update method
void update(float dt)
{
	handleKeys(dt);
	handleMouse(dt);
	
	// check for shader file refresh
	FILETIME create, access, write;
	HANDLE fhandle = CreateFile(SHADER_SRC, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	GetFileTime(fhandle, &create, &access, &write);
	ULONGLONG time = (((ULONGLONG) write.dwHighDateTime) << 32) + write.dwLowDateTime;
	if  (time > g_shader_modified) {
		// detach old stuff
		for(int i = 0; i < g_shader.progs.size(); i++) {
			glDetachShader(g_shader.id, g_shader.progs.at(i));
		}
		g_shader.progs.clear();
		refreshShaderProgram(SHADER_SRC);
		g_shader_modified = time;
	}
	CloseHandle(fhandle);
}

const char g_windowClass[] = "glSandboxWindowClass";

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	switch(msg)
	{
		case WM_CLOSE:
			g_running = false;
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_KEYDOWN:
			keys[frontKeyBuffer][wParam] = true;
			break;
		case WM_KEYUP:
			keys[frontKeyBuffer][wParam] = false;
			break;
		case WM_LBUTTONDOWN:
			mouse[frontKeyBuffer][0] = true;
			break;
		case WM_LBUTTONUP:
			mouse[frontKeyBuffer][0] = false;
			break;
		case WM_RBUTTONDOWN:
			mouse[frontKeyBuffer][2] = true;
			break;
		case WM_RBUTTONUP:
			mouse[frontKeyBuffer][2] = false;
			break;
		case WM_MBUTTONDOWN:
			mouse[frontKeyBuffer][1] = true;
			break;
		case WM_MBUTTONUP:
			mouse[frontKeyBuffer][1] = true;
			break;
		case WM_MOUSEMOVE:
			// x coordinate is lower order short of lParam, y is higher order short
			// onMouseMove(LOWORD(lParam), HIWORD(lParam));
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

long long g_current_millis;

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

	initGL(winWidth, winHeight);
	printf("finished init, starting main loop\n");
	
	g_hwnd = hwnd;
	g_current_millis = currentMillis();
	// main loop
	while(g_running)
	{
		while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			int temp = GetMessage(&msg, NULL, 0, 0);
			if(temp > 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				printf("getmessage returned nonpositive: (%i)\n", temp);
				g_running = false;
				break;
			}
		}
		
		long long current_millis = currentMillis();
		//printf("current_millis is %i\n", current_millis);
		float dt = g_current_millis - current_millis;
		g_current_millis = current_millis;
		
		// draw stuff
		renderGL();
		update(dt);
		g_running &= !keys[frontKeyBuffer][VK_ESCAPE];
		swapKeyBuffers();
		SwapBuffers(hdc);
	}
	printf("quitting\n");
	
	//FreeConsole();
	// delete the rendering context
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(context);
	return msg.wParam;
}