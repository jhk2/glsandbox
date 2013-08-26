#ifndef GLEXTENSIONFUNCS_H
#define GLEXTENSIONFUNCS_H
// define stubs for gl extension functions here (load in *glbase classes)
// TODO: Maybe autogenerate parts of this in the future
#ifdef _WIN32
#include <windows.h>
#endif

#include <gl/gl.h>
#include <gl/glext.h>

// vertex buffers
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;

// indexed buffer objects
extern PFNGLBINDBUFFERBASEPROC glBindBufferBase;

// vertex attributes
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLVERTEXATTRIBFORMATPROC glVertexAttribFormat;
extern PFNGLVERTEXATTRIBBINDINGPROC glVertexAttribBinding;
extern PFNGLBINDVERTEXBUFFERPROC glBindVertexBuffer;

// vertex array objects
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;

// shader compilation/use
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLDETACHSHADERPROC glDetachShader;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLVALIDATEPROGRAMPROC glValidateProgram;
extern PFNGLVALIDATEPROGRAMPIPELINEPROC glValidateProgramPipeline;
// program pipeline objects
extern PFNGLGENPROGRAMPIPELINESPROC glGenProgramPipelines;
extern PFNGLDELETEPROGRAMPIPELINESPROC glDeleteProgramPipelines;
extern PFNGLBINDPROGRAMPIPELINEPROC glBindProgramPipeline;
// uniform variables
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1UIPROC glUniform1ui;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
// uniform matrices
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
// reading back uniforms
extern PFNGLGETUNIFORMUIVPROC glGetUniformuiv;

// frame buffers and the like
extern PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
extern PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer;
extern PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;
extern PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
extern PFNGLFRAMEBUFFERPARAMETERIPROC glFramebufferParameteri;
extern PFNGLDRAWBUFFERSPROC glDrawBuffers;

// renderbuffer stuff, currently unusued
/*
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
*/

// textures
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLTEXIMAGE3DPROC glTexImage3D;
extern PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
// samplers
extern PFNGLGENSAMPLERSPROC glGenSamplers;
extern PFNGLDELETESAMPLERSPROC glDeleteSamplers;
extern PFNGLBINDSAMPLERPROC glBindSampler;
extern PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri;

// color clamping
extern PFNGLCLAMPCOLORPROC glClampColor;

// image units
/*
typedef void (APIENTRYP PFNGLBINDIMAGETEXTUREPROC) (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
typedef void (APIENTRYP PFNGLMEMORYBARRIERPROC) (GLbitfield barriers);
extern PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
extern PFNGLMEMORYBARRIERPROC glMemoryBarrier;
*/
extern PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
extern PFNGLMEMORYBARRIERPROC glMemoryBarrier;

// tessellation
extern PFNGLPATCHPARAMETERIPROC glPatchParameteri;

// compute shader
extern PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;

// debugging with ARB_DEBUG_OUTPUT
extern PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB;

// function to load the extensions
extern void loadGLExtensions();

#endif // GLEXTENSIONLOADER_H
