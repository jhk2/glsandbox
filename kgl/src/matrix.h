#ifndef MATRIX_H
#define MATRIX_H

#include "utils.h"

// class for representing a matrix (presumably to be used by OpenGL in some way)
// most math copied from Lighthouse3D VSML
// http://www.lighthouse3d.com/very-simple-libs

class Matrix
{
public:
    Matrix();
    Matrix(const Matrix &other);

    void multMatrix(const float *other);
    void multMatrix(const Matrix &other);

    void loadMatrix(const float *other);
    void loadIdentity();

    void translate(const float x, const float y, const float z);
    void translate(const fl3 &vec);

    void scale(const float x, const float y, const float z);
    void scale(const fl3 &vec);

    void rotate(const float degrees, const float x, const float y, const float z);
    void rotate(const float degrees, const fl3 &axis);

    // these ops only make sense on either model/view or projection matrices, but including them all here
    void lookAt(const float xPos, const float yPos, const float zPos,
                const float xLook, const float yLook, const float zLook,
                const float xUp, const float yUp, const float zUp);
    void lookAt(const fl3 &pos, const fl3 &look, const fl3 &up);
    void perspective(const float fov, const float ratio, const float nearp, const float farp);
    void ortho(const float left, const float right, const float bottom, const float top, const float nearp=-1.0f, const float farp=1.0f);
    void frustum(const float left, const float right, const float bottom, const float top, const float nearp, const float farp);

    const float* data() const;
    void print();
private:
    float entries_[16];

    void lookAtHelper(const fl3 &pos, const fl3 &dir, const fl3 &up, const fl3 &right);
};

#endif // MATRIX_H
