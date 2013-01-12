#include "glextensionfuncs.h"

#ifdef _WIN32
#define GETPROCADDR wglGetProcAddress
#elif defined __linux__
#define GETPROCADDR glXGetProcAddress
#else
#define GETPROCADDR IDKHowToGetProcAddress
#endif

// matrix operations
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

// vertex buffers
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;

// vertex attributes
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
// vertex array objects
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;

// shader compilation/use
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
// uniform variables
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1UIPROC glUniform1ui;

void loadGLExtensions()
{
	// matrix operations
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) GETPROCADDR("glUniformMatrix4fv");
	
	// vertex buffers
	glBindBuffer = (PFNGLBINDBUFFERPROC) GETPROCADDR("glBindBuffer");
	glGenBuffers = (PFNGLGENBUFFERSPROC) GETPROCADDR("glGenBuffers");
	glBufferData = (PFNGLBUFFERDATAPROC) GETPROCADDR("glBufferData");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) GETPROCADDR("glDeleteBuffers");
	
	// vertex array objects
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) GETPROCADDR("glEnableVertexAttribArray");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) GETPROCADDR("glVertexAttribPointer");
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) GETPROCADDR("glDeleteVertexArrays");
	
	// shder compilation/use
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) GETPROCADDR("glGetShaderInfoLog");
	glCreateShader = (PFNGLCREATESHADERPROC) GETPROCADDR("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC) GETPROCADDR("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC) GETPROCADDR("glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC) GETPROCADDR("glCreateProgram");
	glLinkProgram = (PFNGLLINKPROGRAMPROC) GETPROCADDR("glLinkProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC) GETPROCADDR("glAttachShader");
	glUseProgram = (PFNGLUSEPROGRAMPROC) GETPROCADDR("glUseProgram");
	glDetachShader = (PFNGLDETACHSHADERPROC) GETPROCADDR("glDetachShader");
	glDeleteShader = (PFNGLDELETESHADERPROC) GETPROCADDR("glDeleteShader");
	// uniform variables
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) GETPROCADDR("glGetUniformLocation");
	glUniform1ui = (PFNGLUNIFORM1UIPROC) GETPROCADDR("glGetUniform1ui");
}