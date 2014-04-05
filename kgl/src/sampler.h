#ifndef SAMPLER_H
#define SAMPLER_H
#ifdef _WIN32
#include <windows.h>
#endif
#include <gl/gl.h>

struct Sampler
{
public:
    Sampler() { glGenSamplers(1, &id_); }
    virtual ~Sampler() { glDeleteSamplers(1, &id_); }
    void bind(GLuint texIndex) { glBindSampler(texIndex, id_); }
    GLuint getID() { return id_; }
private:
    GLuint id_;
};

#endif // SAMPLER_H
