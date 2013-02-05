#include "framebuffer.h"
#include "glextensionfuncs.h"
#include <assert.h>
#include <stdio.h>

Framebuffer::Framebuffer(FramebufferParams &params)
{
	params_ = params;
	init();
}

Framebuffer::~Framebuffer()
{
	if (params_.type == GL_TEXTURE_2D) {
		if (params_.numSamples > 0) {
			glDeleteRenderbuffers(params_.numMrts, &color_[0]);
		} else {
			glDeleteTextures(params_.numMrts, &color_[0]);
		}
	} else {
		glDeleteTextures(1, &color_[0]);
	}
	if (params_.depthEnable) {
		if (params_.type == GL_TEXTURE_2D && params_.numSamples > 0) {
			glDeleteRenderbuffers(1, &depth_);
		} else {
			glDeleteTextures(1, &depth_);
		}
	}
	glDeleteFramebuffers(1, &id_);
	delete[] color_;
}

void Framebuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, id_);
}

bool Framebuffer::bindColorTexture(unsigned int idx)
{
	if (params_.numSamples > 0) {
		// an msaa buffer uses renderbuffers and can't be bound to a texture, only drawn into
		return false;
	} else {
		if (params_.type == GL_TEXTURE_2D) {
			glBindTexture(GL_TEXTURE_2D, color_[idx]);
		} else if (params_.type == GL_TEXTURE_3D) {
			glBindTexture(GL_TEXTURE_3D, color_[0]);
		}
		return true;
	}
}

bool Framebuffer::bindDepthTexture()
{
	if (params_.numSamples > 0 || !params_.depthEnable) {
		return false;
	} else {
		glBindTexture(GL_TEXTURE_2D, depth_);
		return true;
	}
}

bool Framebuffer::init(bool gen)
{
	//~ printf("init framebuffer with gen %u\n", gen); fflush(stdout);
	if (gen) {
		glGenFramebuffers(1, &id_);
	}
	
	if (params_.numMrts == 0) {
		printf("num mrts is zero, failed!\n"); fflush(stdout);
		return false;
	}
	if (params_.depth > GL_MAX_COLOR_ATTACHMENTS) { // same for mrt
		printf("num mrts is too many, failed\n"); fflush(stdout);
		return false;
	}
	
	bind();
	
	if (gen) {
		if (params_.type == GL_TEXTURE_2D) {
			color_ = new GLuint[params_.numMrts];
		} else if (params_.type == GL_TEXTURE_3D) {
			color_ = new GLuint[1];
		}
	}
	
	if (params_.numSamples > 0) { // MSAA
		//~ printf("making msaa renderbuffers\n"); fflush(stdout);
		GLint maxSamples = 0;
		glGetIntegerv(GL_MAX_SAMPLES_EXT, &maxSamples);
		if (params_.numSamples > maxSamples) {
			params_.numSamples = static_cast<GLuint>(maxSamples);
		}
		if (params_.type == GL_TEXTURE_2D) {
			if (gen) {
				printf("making msaa buffer with %u samples\n", params_.numSamples); fflush(stdout);
				glGenRenderbuffers(params_.numMrts, &color_[0]);
			}
			for (int i = 0; i < params_.numMrts; i++) {
				glBindRenderbuffer(GL_RENDERBUFFER, color_[i]);
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, params_.numSamples, params_.format, params_.width, params_.height);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER, color_[i]);
			}
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			// todo, maybe add renderbuffer version of 2d
		} else {
			printf("can't make 3D MSAA framebuffer\n"); fflush(stdout);
		}
	} else { // non-MSAA normal
		if (params_.type == GL_TEXTURE_2D) {
			//~ printf("making non-msaa 2d framebuffer\n"); fflush(stdout);
			if (gen) {
				glGenTextures(params_.numMrts, &color_[0]);
			}
			for (int i = 0; i < params_.numMrts; i++) {
				glBindTexture(GL_TEXTURE_2D, color_[i]);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params_.filter);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params_.filter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				glTexImage2D(GL_TEXTURE_2D, 0, params_.format, params_.width, params_.height, 0, GL_RGBA, GL_FLOAT, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_[i], 0);
			}
			glBindTexture(params_.type, 0);
		} else if (params_.type == GL_TEXTURE_3D) {
			//~ printf("making non-msaa 3d framebuffer\n"); fflush(stdout);
			if (gen) {
				glGenTextures(1, &color_[0]);
			}
			glBindTexture(GL_TEXTURE_3D, color_[0]);
			glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // must be linear or nearest
			glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage3D(GL_TEXTURE_3D, 0, params_.format, params_.width, params_.height, params_.depth, 0, GL_RGBA, GL_FLOAT, 0);
			for (int i = 0; i < params_.depth; i++) {
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color_[0], 0, i);
			}
			glBindTexture(params_.type, 0);
		}
	}
	
	if(params_.depthEnable) {
		if (params_.numSamples > 0) {
			//~ printf("making msaa depth buffer\n"); fflush(stdout);
			if (gen) {
				glGenRenderbuffers(1, &depth_);
			}
			glBindRenderbuffer(GL_RENDERBUFFER, depth_);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, params_.numSamples, params_.depthFormat, params_.width, params_.height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		} else {
			//~ printf("making non-msaa depth buffer\n"); fflush(stdout);
			if (gen) {
				glGenTextures(1, &depth_);
			}
			glBindTexture(GL_TEXTURE_2D, depth_);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			// glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT); // should be default
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, params_.depthFormat, params_.width, params_.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

void Framebuffer::blit(Framebuffer &dest) {
	assert(params_.numMrts == dest.params_.numMrts);
	
	for (int i = 0; i < params_.numMrts; i++) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
		glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest.id_);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
		glBlitFramebuffer(0, 0, params_.width, params_.height, 0, 0, dest.params_.width, dest.params_.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}
	
	if (params_.depthEnable && dest.params_.depthEnable) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest.id_);
		glBlitFramebuffer(0, 0, params_.width, params_.height, 0, 0, dest.params_.width, dest.params_.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	}
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Framebuffer::resize(GLuint width, GLuint height)
{
	if (params_.width == width && params_.height == height) {
		return;
	}
	params_.width = width;
	params_.height = height;
	init(false);
}