#include "glextensionfuncs.h"

#ifdef _WIN32
#define GETPROCADDR wglGetProcAddress
#elif defined __linux__
#define GETPROCADDR glXGetProcAddress
#else
#define GETPROCADDR IDKHowToGetProcAddress
#endif

// vertex buffers
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;

// indexed buffer objects (for uniform buffers and transform feedback)
PFNGLBINDBUFFERBASEPROC glBindBufferBase;

// texture buffer objects
PFNGLTEXBUFFERRANGEPROC glTexBufferRange;

// vertex attributes
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLVERTEXATTRIBFORMATPROC glVertexAttribFormat;
PFNGLVERTEXATTRIBBINDINGPROC glVertexAttribBinding;
PFNGLBINDVERTEXBUFFERPROC glBindVertexBuffer;

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
PFNGLVALIDATEPROGRAMPROC glValidateProgram;
PFNGLVALIDATEPROGRAMPIPELINEPROC glValidateProgramPipeline;
// program pipeline objects
PFNGLGENPROGRAMPIPELINESPROC glGenProgramPipelines;
PFNGLDELETEPROGRAMPIPELINESPROC glDeleteProgramPipelines;
PFNGLBINDPROGRAMPIPELINEPROC glBindProgramPipeline;
// uniform variables
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1UIPROC glUniform1ui;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
// matrix operations
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
// reading back uniforms
PFNGLGETUNIFORMUIVPROC glGetUniformuiv;

// MSAA/SSAA
PFNGLMINSAMPLESHADINGPROC glMinSampleShading;

// frame buffers and the like
PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer;
PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLFRAMEBUFFERPARAMETERIPROC glFramebufferParameteri;
PFNGLDRAWBUFFERSPROC glDrawBuffers;

// renderbuffer stuff, currently unusued
/*
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
*/

// textures
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLTEXIMAGE3DPROC glTexImage3D;
PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
// samplers
PFNGLGENSAMPLERSPROC glGenSamplers;
PFNGLDELETESAMPLERSPROC glDeleteSamplers;
PFNGLBINDSAMPLERPROC glBindSampler;
PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri;

PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
PFNGLMEMORYBARRIERPROC glMemoryBarrier;

// tessellation
PFNGLPATCHPARAMETERIPROC glPatchParameteri;

// compute shader
PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;

// color clamping
PFNGLCLAMPCOLORPROC glClampColor;

// debugging with ARB_DEBUG_OUTPUT
PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB;

void loadGLExtensions()
{	
	// vertex buffers
	glBindBuffer = (PFNGLBINDBUFFERPROC) GETPROCADDR("glBindBuffer");
	glGenBuffers = (PFNGLGENBUFFERSPROC) GETPROCADDR("glGenBuffers");
	glBufferData = (PFNGLBUFFERDATAPROC) GETPROCADDR("glBufferData");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC) GETPROCADDR("glBufferSubData");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) GETPROCADDR("glDeleteBuffers");
	
    // indexed buffer objects (for uniform buffers and transform feedback)
	glBindBufferBase = (PFNGLBINDBUFFERBASEPROC) GETPROCADDR("glBindBufferBase");

    // texture buffer objects
    glTexBufferRange = (PFNGLTEXBUFFERRANGEPROC) GETPROCADDR("glTexBufferRange");
	
	// vertex attributes
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) GETPROCADDR("glEnableVertexAttribArray");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) GETPROCADDR("glVertexAttribPointer");
	glVertexAttribFormat = (PFNGLVERTEXATTRIBFORMATPROC) GETPROCADDR("glVertexAttribFormat");
	glVertexAttribBinding = (PFNGLVERTEXATTRIBBINDINGPROC) GETPROCADDR("glVertexAttribBinding");
	glBindVertexBuffer = (PFNGLBINDVERTEXBUFFERPROC) GETPROCADDR("glBindVertexBuffer");
	
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
	glValidateProgram = (PFNGLVALIDATEPROGRAMPROC) GETPROCADDR("glValidateProgram");
	glValidateProgramPipeline = (PFNGLVALIDATEPROGRAMPIPELINEPROC) GETPROCADDR("glValidateProgramPipeline");
	// program pipeline objects
	glGenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC) GETPROCADDR("glGenProgramPipelines");
	glDeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC) GETPROCADDR("glDeleteProgramPipelines");
	glBindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC) GETPROCADDR("glBindProgramPipeline");
	// uniform variables
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) GETPROCADDR("glGetUniformLocation");
	glUniform1ui = (PFNGLUNIFORM1UIPROC) GETPROCADDR("glUniform1ui");
	glUniform1i = (PFNGLUNIFORM1IPROC) GETPROCADDR("glUniform1i");
	glUniform1f = (PFNGLUNIFORM1FPROC) GETPROCADDR("glUniform1f");
	glUniform3fv = (PFNGLUNIFORM3FVPROC) GETPROCADDR("glUniform3fv");
	glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC) GETPROCADDR("glGetUniformBlockIndex");
	// matrix operations
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) GETPROCADDR("glUniformMatrix4fv");
	// reading back uniforms
	glGetUniformuiv = (PFNGLGETUNIFORMUIVPROC) GETPROCADDR("glGetUniformuiv");

    // MSAA/SSAA
    glMinSampleShading = (PFNGLMINSAMPLESHADINGPROC) GETPROCADDR("glMinSampleShading");
	
	// frame buffers and the like
	glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC) GETPROCADDR("glBindFragDataLocation");
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) GETPROCADDR("glGenFramebuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) GETPROCADDR("glBindFramebuffer");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) GETPROCADDR("glDeleteFramebuffers");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) GETPROCADDR("glFramebufferTexture2D");
	glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC) GETPROCADDR("glFramebufferTextureLayer");
	glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC) GETPROCADDR("glFramebufferTexture");
	glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC) GETPROCADDR("glBlitFramebuffer");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) GETPROCADDR("glCheckFramebufferStatus");
	glFramebufferParameteri = (PFNGLFRAMEBUFFERPARAMETERIPROC) GETPROCADDR("glFramebufferParameteri");
    glDrawBuffers = (PFNGLDRAWBUFFERSPROC) GETPROCADDR("glDrawBuffers");

	// renderbuffers (unused)
	/*
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) GETPROCADDR("glGenRenderbuffers");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) GETPROCADDR("glBindRenderbuffer");
	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) GETPROCADDR("glDeleteRenderbuffers");
	glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) GETPROCADDR("glRenderbufferStorageMultisample");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) GETPROCADDR("glFramebufferRenderbuffer");
	*/
	
	// textures
	glActiveTexture = (PFNGLACTIVETEXTUREPROC) GETPROCADDR("glActiveTexture");
	glTexImage3D = (PFNGLTEXIMAGE3DPROC) GETPROCADDR("glTexImage3D");
	glTexImage2DMultisample = (PFNGLTEXIMAGE2DMULTISAMPLEPROC) GETPROCADDR("glTexImage2DMultisample");
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) GETPROCADDR("glGenerateMipmap");
	// samplers
	glGenSamplers = (PFNGLGENSAMPLERSPROC) GETPROCADDR("glGenSamplers");
	glDeleteSamplers = (PFNGLDELETESAMPLERSPROC) GETPROCADDR("glDeleteSamplers");
	glBindSampler = (PFNGLBINDSAMPLERPROC) GETPROCADDR("glBindSampler");
	glSamplerParameteri = (PFNGLSAMPLERPARAMETERIPROC) GETPROCADDR("glSamplerParameteri");
	
	// image units
	glBindImageTexture = (PFNGLBINDIMAGETEXTUREPROC) GETPROCADDR("glBindImageTexture");
	glMemoryBarrier = (PFNGLMEMORYBARRIERPROC) GETPROCADDR("glMemoryBarrier");
	
	// tessellation
	glPatchParameteri = (PFNGLPATCHPARAMETERIPROC) GETPROCADDR("glPatchParameteri");

    // compute shader
    glDispatchCompute = (PFNGLDISPATCHCOMPUTEPROC) GETPROCADDR("glDispatchCompute");
	
	// color clamping
	glClampColor = (PFNGLCLAMPCOLORPROC) GETPROCADDR("glClampColor");
	
	// debugging with ARB_DEBUG_OUTPUT
	glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC) GETPROCADDR("glDebugMessageCallbackARB");
}
