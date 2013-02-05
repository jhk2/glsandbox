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
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLDETACHSHADERPROC glDetachShader;
PFNGLDELETESHADERPROC glDeleteShader;
// uniform variables
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1UIPROC glUniform1ui;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM3FVPROC glUniform3fv;

// frame buffers and the like
PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;

// textures
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLTEXIMAGE3DPROC glTexImage3D;

void loadGLExtensions()
{
	// matrix operations
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) GETPROCADDR("glUniformMatrix4fv");
	
	// vertex buffers
	glBindBuffer = (PFNGLBINDBUFFERPROC) GETPROCADDR("glBindBuffer");
	glGenBuffers = (PFNGLGENBUFFERSPROC) GETPROCADDR("glGenBuffers");
	glBufferData = (PFNGLBUFFERDATAPROC) GETPROCADDR("glBufferData");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) GETPROCADDR("glDeleteBuffers");
	
	// vertex attributes
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) GETPROCADDR("glEnableVertexAttribArray");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) GETPROCADDR("glVertexAttribPointer");
	// vertex array objects
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) GETPROCADDR("glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) GETPROCADDR("glBindVertexArray");
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) GETPROCADDR("glDeleteVertexArrays");
	
	// shader compilation/use
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) GETPROCADDR("glGetShaderInfoLog");
	glCreateShader = (PFNGLCREATESHADERPROC) GETPROCADDR("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC) GETPROCADDR("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC) GETPROCADDR("glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC) GETPROCADDR("glCreateProgram");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC) GETPROCADDR("glDeleteProgram");
	glLinkProgram = (PFNGLLINKPROGRAMPROC) GETPROCADDR("glLinkProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC) GETPROCADDR("glAttachShader");
	glUseProgram = (PFNGLUSEPROGRAMPROC) GETPROCADDR("glUseProgram");
	glDetachShader = (PFNGLDETACHSHADERPROC) GETPROCADDR("glDetachShader");
	glDeleteShader = (PFNGLDELETESHADERPROC) GETPROCADDR("glDeleteShader");
	// uniform variables
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) GETPROCADDR("glGetUniformLocation");
	glUniform1ui = (PFNGLUNIFORM1UIPROC) GETPROCADDR("glUniform1ui");
	glUniform1f = (PFNGLUNIFORM1FPROC) GETPROCADDR("glUniform1f");
	glUniform3fv = (PFNGLUNIFORM3FVPROC) GETPROCADDR("glUniform3fv");
	
	// frame buffers and the like
	glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC) GETPROCADDR("glBindFragDataLocation");
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) GETPROCADDR("glGenFramebuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) GETPROCADDR("glBindFramebuffer");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) GETPROCADDR("glDeleteFramebuffers");
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) GETPROCADDR("glGenRenderbuffers");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) GETPROCADDR("glBindRenderbuffer");
	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) GETPROCADDR("glDeleteRenderbuffers");
	glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) GETPROCADDR("glRenderbufferStorageMultisample");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) GETPROCADDR("glFramebufferRenderbuffer");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) GETPROCADDR("glFramebufferTexture2D");
	glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC) GETPROCADDR("glFramebufferTextureLayer");
	glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC) GETPROCADDR("glBlitFramebuffer");
	
	// textures
	glActiveTexture = (PFNGLACTIVETEXTUREPROC) GETPROCADDR("glActiveTexture");
	glTexImage3D = (PFNGLTEXIMAGE3DPROC) GETPROCADDR("glTexImage3D");
}