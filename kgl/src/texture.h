#ifndef TEXTURE_H
#define TEXTURE_H
#ifdef _WIN32
#include <windows.h>
#endif
#include <gl/gl.h>
#include "utils.h"

/*
*	Class for representing textures
*	We assume 2D textures only for this, since we are loading from image files
*	TODO: add mipmaps and jpegs
*/

class Texture
{
	public:
		Texture();
		Texture(const Texture &other);
		Texture(const char *filename);
		virtual ~Texture();
		void bind(GLenum target = GL_TEXTURE_2D);
		void bindToImage(GLuint unit, GLenum access, GLenum format);
		GLuint getID();
	private:
		bool init(const char *filename);
		bool loadPng(const char *filename);
		bool loadJpg(const char *filename);
		void sendGL(const GLvoid *data);

		GLuint id_;
		int2 dims_;
		bool original_;
};
#endif // TEXTURE_H