#ifndef DEBUG_H
#define DEBUG_H

#ifdef _WIN32
#include <windows.h>
#endif
#include <gl/gl.h>
#include <stdio.h>
#include <map>

// some debugging stuff
static void checkGLError(const char *function)
{
    GLenum err = glGetError();
    switch(err) {
    case GL_NO_ERROR: printf("no error at function %s\n", function); fflush(stdout); break;
    case GL_INVALID_ENUM: printf("invalid enum error at function %s\n", function); fflush(stdout); break;
    case GL_INVALID_VALUE: printf("invalid value error at function %s\n", function); fflush(stdout); break;
    case GL_INVALID_OPERATION: printf("invalid operation error at function$s\n", function); fflush(stdout); break;
    default: printf("other error\n"); fflush(stdout); break;
    }
}

static void APIENTRY errorCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, void *userParam)
{
    printf("GL Debug Error: %s\n", message); fflush(stdout);
}

#endif // DEBUG_H
