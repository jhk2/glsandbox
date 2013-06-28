#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#ifdef _WIN32
#include <windows.h>
#endif
#include <gl/gl.h>
#include "utils.h"

// initialization parameters
struct FramebufferParams {
	//~ FramebufferParams(const FramebufferParams &other) 
		//~ : width(other.width), height(other.height), numSamples(other.numsamples), depth(other.depth), depthEnable(other.depthEnable), 
		//~ format(other.format), depthFormat(other.depthFormat), type(other.type), filter(other.filter) {}
	GLuint width, height, numSamples; // numCsamples
	union {
		GLuint numMrts;
		GLuint depth;
	};
	bool colorEnable;
	bool depthEnable;
	GLenum format, depthFormat, type, filter;
};

class Framebuffer
{
	public:
		Framebuffer(FramebufferParams &params);
		virtual ~Framebuffer();
	
		void bind();
		bool bindColorTexture(unsigned int idx = 0);
		bool bindDepthTexture();
		void blit(Framebuffer &dest);
		void blit(); // blits to default framebuffer
		void resize(GLuint width, GLuint height);

        // dump contents to output
        void dumpColor(int2 start, int2 size, GLenum format, GLenum type) const;
        void dumpDepth(int2 start, int2 size, GLenum format, GLenum type) const;

		GLuint getWidth();
		GLuint getHeight();
		GLuint getColorID(unsigned int idx);
		GLuint getDepthID();
	protected:
		bool init(bool gen = true);
		bool checkStatus(GLenum target);
	
		GLuint depth_, *color_, id_;
		FramebufferParams params_;
};
#endif // FRAMEBUFFER_H
