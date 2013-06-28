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
    if (params_.colorEnable) {
        glDeleteTextures(params_.numMrts, &color_[0]);
    }
    if (params_.depthEnable) {
        glDeleteTextures(1, &depth_);
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
    if (params_.type == GL_TEXTURE_2D || params_.type == GL_TEXTURE_2D_MULTISAMPLE) {
        glBindTexture(params_.type, color_[idx]);
    } else if (params_.type == GL_TEXTURE_3D) {
        glBindTexture(GL_TEXTURE_3D, color_[0]);
    }
    return true;
}

bool Framebuffer::bindDepthTexture()
{
    if (!params_.depthEnable) {
        return false;
    }else if (params_.type == GL_TEXTURE_2D_MULTISAMPLE) {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depth_);
    } else {
        glBindTexture(GL_TEXTURE_2D, depth_);
    }
    return true;
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

    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, params_.width);
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, params_.height);

    if (params_.colorEnable) {
        if (gen) {
            color_ = new GLuint[params_.numMrts];
        }

        if (params_.type == GL_TEXTURE_2D) {
            //~ printf("making non-msaa 2d framebuffer\n"); fflush(stdout);
            if (gen) {
                glGenTextures(params_.numMrts, &color_[0]);
            }
            for (unsigned int i = 0; i < params_.numMrts; i++) {
                //~ printf("making non msaa color target for id %u\n", color_[i]); fflush(stdout);
                glBindTexture(GL_TEXTURE_2D, color_[i]);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params_.filter);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params_.filter);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                glTexImage2D(GL_TEXTURE_2D, 0, params_.format, params_.width, params_.height, 0, GL_RGBA, GL_FLOAT, 0);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_[i], 0);
            }
        } else if (params_.type == GL_TEXTURE_2D_MULTISAMPLE) {
            GLint maxSamples = 0;
            glGetIntegerv(GL_MAX_SAMPLES_EXT, &maxSamples);
            params_.numSamples = min(params_.numSamples, static_cast<GLuint>(maxSamples));
            if (gen) {
                glGenTextures(params_.numMrts, &color_[0]);
            }
            for (int i = 0; i < params_.numMrts; i++) {
                //~ printf("making msaa color target for id %u\n", color_[i]); fflush(stdout);
                glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, color_[i]);
                //~ glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                //~ glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                //~ glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                //~ glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, params_.numSamples, params_.format , params_.width, params_.height, false);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, color_[i], 0);
            }
        } else if (params_.type == GL_TEXTURE_3D) {
            //~ printf("making non-msaa 3d framebuffer\n"); fflush(stdout);
            if (gen) {
                glGenTextures(params_.numMrts, &color_[0]);
            }
            for (int i = 0; i < params_.numMrts; i++) {
                glBindTexture(GL_TEXTURE_3D, color_[i]);
                glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // must be linear or nearest
                glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexImage3D(GL_TEXTURE_3D, 0, params_.format, params_.width, params_.height, params_.depth, 0, GL_RGBA, GL_FLOAT, 0);
                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color_[i], 0);
            }
            /* binding each layer to a color attachment (old way)
            for (int i = 0; i < params_.depth; i++) {
                glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color_[0], 0, i);
            }
            */
        }
        GLenum *bufs = new GLenum[params_.numMrts];
        for (unsigned int i = 0; i < params_.numMrts; i++) {
            bufs[i] = GL_COLOR_ATTACHMENT0 + i;
        }
        glDrawBuffers(params_.numMrts, bufs);
        delete[] bufs;
        glBindTexture(params_.type, 0);
    }

    if(params_.depthEnable) {
        if (gen) {
            glGenTextures(1, &depth_);
        }
        if (params_.type == GL_TEXTURE_2D_MULTISAMPLE) {
            //~ printf("making msaa z target id for %u\n", depth_); fflush(stdout);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depth_);
            //~ glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            //~ glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            //~ glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            //~ glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // this was throwing GL debug errors, so maybe take care of it in a sampler, or when we are doing shadow mapping?
            //~ glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            //~ glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, params_.numSamples, params_.depthFormat, params_.width, params_.height, false);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth_, 0);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        } else if (params_.type == GL_TEXTURE_3D) {
            glBindTexture(GL_TEXTURE_3D, depth_);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
            glTexImage3D(GL_TEXTURE_3D, 0, params_.depthFormat, params_.width, params_.height, params_.depth, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_, 0);
            glBindTexture(GL_TEXTURE_3D, 0);
        } else {
            //~ printf("making normal z target for id %u\n", depth_); fflush(stdout);
            glBindTexture(GL_TEXTURE_2D, depth_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            // glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT); // should be default
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            // comparison functions for shadow mapping
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
            glTexImage2D(GL_TEXTURE_2D, 0, params_.depthFormat, params_.width, params_.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    bool check = checkStatus(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return check;
}

void Framebuffer::blit(Framebuffer &dest)
{
    assert(params_.numMrts == dest.params_.numMrts);

    if (params_.colorEnable && dest.params_.colorEnable) {
        for (int i = 0; i < params_.numMrts; i++) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
            glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest.id_);
            glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
            glBlitFramebuffer(0, 0, params_.width, params_.height, 0, 0, dest.params_.width, dest.params_.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
    }

    if (params_.depthEnable && dest.params_.depthEnable) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest.id_);
        glBlitFramebuffer(0, 0, params_.width, params_.height, 0, 0, dest.params_.width, dest.params_.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Framebuffer::blit()
{
    if (params_.colorEnable) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(0, 0, params_.width, params_.height, 0, 0, params_.width, params_.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    if (params_.depthEnable) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, params_.width, params_.height, 0, 0, params_.width, params_.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
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

bool Framebuffer::checkStatus(GLenum target) {
    GLuint status = glCheckFramebufferStatus(target);
    switch(status) {
    case GL_FRAMEBUFFER_COMPLETE:
        printf("framebuffer check ok\n"); fflush(stdout);
        return true;
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        printf("framebuffer incomplete attachment\n"); fflush(stdout);
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        printf("framebuffer missing attachment\n"); fflush(stdout);
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        printf("framebuffer incomplete dimensions\n"); fflush(stdout);
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        printf("framebuffer incomplete formats\n"); fflush(stdout);
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        printf("framebuffer incomplete draw buffer\n"); fflush(stdout);
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        printf("framebuffer incomplete read buffer\n"); fflush(stdout);
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        printf("framebuffer incomplete multisample\n"); fflush(stdout);
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS :
        printf("framebuffer incomplete layer targets\n"); fflush(stdout);
        break;

    case GL_FRAMEBUFFER_UNSUPPORTED:
        printf("framebuffer unsupported internal format or image\n"); fflush(stdout);
        break;

    default:
        printf("other framebuffer error\n"); fflush(stdout);
        break;
    }

    return false;
}

void Framebuffer::dumpColor(int2 start, int2 size, GLenum format, GLenum type) const
{
    // only support dumping floats for now
    assert(type == GL_FLOAT);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
    const unsigned int rectSize = size.x * size.y;
    if (params_.colorEnable) {
        // resize dest to be correct size
        unsigned int numComponents;
        switch (format) {
        case GL_RED:
        case GL_BLUE:
        case GL_GREEN:
            numComponents = 1;
            break;
        case GL_RGB:
        case GL_BGR:
            numComponents = 3;
            break;
        case GL_RGBA:
        case GL_BGRA:
            numComponents = 4;
            break;
        }
        float *dest = new float[numComponents * rectSize];

        for (unsigned int i = 0; i < params_.numMrts; i++) {
            // dump each color target
            glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
            glReadPixels(start.x, start.y, size.x, size.y, format, type, dest);

            printf("dumping color target index %i\n", i);
            for (unsigned int r = 0; r < size.y; r++) {
                for (unsigned int c = 0; c < size.x; c++) {
                    const unsigned int pixelOffset = numComponents * (r * size.x + c);
                    switch (numComponents) {
                    case 1:
                        printf("(%g)  ", dest[pixelOffset]);
                        break;
                    case 2:
                        printf("(%g, %g)  ", dest[pixelOffset], dest[pixelOffset+1]);
                        break;
                    case 3:
                        printf("(%g, %g, %g)  ", dest[pixelOffset], dest[pixelOffset+1], dest[pixelOffset+2]);
                        break;
                    case 4:
                        printf("(%g, %g, %g, %g)  ", dest[pixelOffset], dest[pixelOffset+1], dest[pixelOffset+2], dest[pixelOffset+3]);
                        break;
                    }
                }
                printf("\n");
            }
            printf("\n");
        }
        printf("\n");

        delete[] dest;
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void Framebuffer::dumpDepth(int2 start, int2 size, GLenum format, GLenum type) const
{
    // only support dumping floats for now
    assert(type == GL_FLOAT);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
    const unsigned int rectSize = size.x * size.y;
    glReadBuffer(GL_FRONT);
    if (params_.depthEnable) {
        // dump depth target
        float *dest = new float[rectSize];

        glReadPixels(start.x, start.y, size.x, size.y, format, type, dest);

        printf("dumping depth target\n");
        for (unsigned int r = 0; r < size.y; r++) {
            for (unsigned int c = 0; c < size.x; c++) {
                const unsigned int pixelOffset = (r * size.x + c);
                if (type == GL_FLOAT) {
                    printf("(%g)  ", dest[pixelOffset]);
                } else {
                    printf("(%i)  ", dest[pixelOffset]);
                }
            }
            printf("\n");
        }
        printf("\n");
        delete[] dest;
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

GLuint Framebuffer::getWidth()
{
    return params_.width;
}

GLuint Framebuffer::getHeight()
{
    return params_.height;
}

GLuint Framebuffer::getColorID(unsigned int idx)
{
    assert(idx < params_.numMrts);
    return color_[idx];
}

GLuint Framebuffer::getDepthID()
{
    return depth_;
}
