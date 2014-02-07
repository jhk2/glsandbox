#include "matrix.h"
#include "constants.h"
#include <stdio.h>
#include <memory.h>

void setIdentityMatrix(float *mat)
{
    for (int i = 0; i < 16; i++) {
        mat[i] = 0.0f;
    }
    for (int i = 0; i < 4; i++) {
        mat[i * 4 + i] = 1.0f;
    }
}

Matrix::Matrix()
{
    loadIdentity();
}

Matrix::Matrix(const Matrix &other)
{
    loadMatrix(other.data());
}

fl3 Matrix::multiplyPoint(const fl3 &pt) const
{
    fl3 output;
    output.x = entries_[0]*pt.x + entries_[4]*pt.y + entries_[8]*pt.z + entries_[12];
    output.y = entries_[1]*pt.x + entries_[5]*pt.y + entries_[9]*pt.z + entries_[13];
    output.z = entries_[2]*pt.x + entries_[6]*pt.y + entries_[10]*pt.z + entries_[14];
    float w = entries_[3]*pt.x + entries_[7]*pt.y + entries_[11]*pt.z + entries_[15];
    output /= w; // returns the homogenized point (w=1)
    return output;
}

fl3 Matrix::multiplyVector(const fl3 &vec) const
{
    // vec is assumed to be a column vector to be dot product-ed with each matrix row
    // output is another column vector
    fl3 output;
    output.x = entries_[0]*vec.x + entries_[4]*vec.y + entries_[8]*vec.z;
    output.y = entries_[1]*vec.x + entries_[5]*vec.y + entries_[9]*vec.z;
    output.z = entries_[2]*vec.x + entries_[6]*vec.y + entries_[10]*vec.z;
    return output;
}

// just a copy of gluInvertMatrix
bool Matrix::getInverse(Matrix &out) const
{
    out.entries_[0] = entries_[5]  * entries_[10] * entries_[15] -
            entries_[5]  * entries_[11] * entries_[14] -
            entries_[9]  * entries_[6]  * entries_[15] +
            entries_[9]  * entries_[7]  * entries_[14] +
            entries_[13] * entries_[6]  * entries_[11] -
            entries_[13] * entries_[7]  * entries_[10];

    out.entries_[4] = -entries_[4]  * entries_[10] * entries_[15] +
            entries_[4]  * entries_[11] * entries_[14] +
            entries_[8]  * entries_[6]  * entries_[15] -
            entries_[8]  * entries_[7]  * entries_[14] -
            entries_[12] * entries_[6]  * entries_[11] +
            entries_[12] * entries_[7]  * entries_[10];

    out.entries_[8] = entries_[4]  * entries_[9] * entries_[15] -
            entries_[4]  * entries_[11] * entries_[13] -
            entries_[8]  * entries_[5] * entries_[15] +
            entries_[8]  * entries_[7] * entries_[13] +
            entries_[12] * entries_[5] * entries_[11] -
            entries_[12] * entries_[7] * entries_[9];

    out.entries_[12] = -entries_[4]  * entries_[9] * entries_[14] +
            entries_[4]  * entries_[10] * entries_[13] +
            entries_[8]  * entries_[5] * entries_[14] -
            entries_[8]  * entries_[6] * entries_[13] -
            entries_[12] * entries_[5] * entries_[10] +
            entries_[12] * entries_[6] * entries_[9];

    out.entries_[1] = -entries_[1]  * entries_[10] * entries_[15] +
            entries_[1]  * entries_[11] * entries_[14] +
            entries_[9]  * entries_[2] * entries_[15] -
            entries_[9]  * entries_[3] * entries_[14] -
            entries_[13] * entries_[2] * entries_[11] +
            entries_[13] * entries_[3] * entries_[10];

    out.entries_[5] = entries_[0]  * entries_[10] * entries_[15] -
            entries_[0]  * entries_[11] * entries_[14] -
            entries_[8]  * entries_[2] * entries_[15] +
            entries_[8]  * entries_[3] * entries_[14] +
            entries_[12] * entries_[2] * entries_[11] -
            entries_[12] * entries_[3] * entries_[10];

    out.entries_[9] = -entries_[0]  * entries_[9] * entries_[15] +
            entries_[0]  * entries_[11] * entries_[13] +
            entries_[8]  * entries_[1] * entries_[15] -
            entries_[8]  * entries_[3] * entries_[13] -
            entries_[12] * entries_[1] * entries_[11] +
            entries_[12] * entries_[3] * entries_[9];

    out.entries_[13] = entries_[0]  * entries_[9] * entries_[14] -
            entries_[0]  * entries_[10] * entries_[13] -
            entries_[8]  * entries_[1] * entries_[14] +
            entries_[8]  * entries_[2] * entries_[13] +
            entries_[12] * entries_[1] * entries_[10] -
            entries_[12] * entries_[2] * entries_[9];

    out.entries_[2] = entries_[1]  * entries_[6] * entries_[15] -
            entries_[1]  * entries_[7] * entries_[14] -
            entries_[5]  * entries_[2] * entries_[15] +
            entries_[5]  * entries_[3] * entries_[14] +
            entries_[13] * entries_[2] * entries_[7] -
            entries_[13] * entries_[3] * entries_[6];

    out.entries_[6] = -entries_[0]  * entries_[6] * entries_[15] +
            entries_[0]  * entries_[7] * entries_[14] +
            entries_[4]  * entries_[2] * entries_[15] -
            entries_[4]  * entries_[3] * entries_[14] -
            entries_[12] * entries_[2] * entries_[7] +
            entries_[12] * entries_[3] * entries_[6];

    out.entries_[10] = entries_[0]  * entries_[5] * entries_[15] -
            entries_[0]  * entries_[7] * entries_[13] -
            entries_[4]  * entries_[1] * entries_[15] +
            entries_[4]  * entries_[3] * entries_[13] +
            entries_[12] * entries_[1] * entries_[7] -
            entries_[12] * entries_[3] * entries_[5];

    out.entries_[14] = -entries_[0]  * entries_[5] * entries_[14] +
            entries_[0]  * entries_[6] * entries_[13] +
            entries_[4]  * entries_[1] * entries_[14] -
            entries_[4]  * entries_[2] * entries_[13] -
            entries_[12] * entries_[1] * entries_[6] +
            entries_[12] * entries_[2] * entries_[5];

    out.entries_[3] = -entries_[1] * entries_[6] * entries_[11] +
            entries_[1] * entries_[7] * entries_[10] +
            entries_[5] * entries_[2] * entries_[11] -
            entries_[5] * entries_[3] * entries_[10] -
            entries_[9] * entries_[2] * entries_[7] +
            entries_[9] * entries_[3] * entries_[6];

    out.entries_[7] = entries_[0] * entries_[6] * entries_[11] -
            entries_[0] * entries_[7] * entries_[10] -
            entries_[4] * entries_[2] * entries_[11] +
            entries_[4] * entries_[3] * entries_[10] +
            entries_[8] * entries_[2] * entries_[7] -
            entries_[8] * entries_[3] * entries_[6];

    out.entries_[11] = -entries_[0] * entries_[5] * entries_[11] +
            entries_[0] * entries_[7] * entries_[9] +
            entries_[4] * entries_[1] * entries_[11] -
            entries_[4] * entries_[3] * entries_[9] -
            entries_[8] * entries_[1] * entries_[7] +
            entries_[8] * entries_[3] * entries_[5];

    out.entries_[15] = entries_[0] * entries_[5] * entries_[10] -
            entries_[0] * entries_[6] * entries_[9] -
            entries_[4] * entries_[1] * entries_[10] +
            entries_[4] * entries_[2] * entries_[9] +
            entries_[8] * entries_[1] * entries_[6] -
            entries_[8] * entries_[2] * entries_[5];

    const float det = entries_[0] * out.entries_[0] + entries_[1] * out.entries_[4] + entries_[2] * out.entries_[8] + entries_[3] * out.entries_[12];

    if (det == 0) {
        return false;
    }

    const float invdet = 1.0f / det;

    for (int i = 0; i < 16; i++) {
        out.entries_[i] = out.entries_[i] * invdet;
    }

    return true;
}

void Matrix::multMatrix(const float *other)
{
    float res[16];
    const float *a = entries_;
    const float *b = other;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            res[j*4 + i] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                res[j*4 + i] += a[k*4 + i] * b[j*4 + k];
            }
        }
    }
    memcpy(entries_, res, 16 * sizeof(float));
}

void Matrix::multMatrix(const Matrix &other)
{
    multMatrix(other.data());
}

void Matrix::loadMatrix(const float *other)
{
    memcpy(entries_, other, 16 * sizeof(float));
}

void Matrix::loadIdentity()
{
    setIdentityMatrix(entries_);
}

void Matrix::translate(const float x, const float y, const float z)
{
    float mat[16];
    setIdentityMatrix(mat);
    mat[12] = x;
    mat[13] = y;
    mat[14] = z;

    multMatrix(mat);
}

void Matrix::translate(const fl3 &vec)
{
    translate(vec.x, vec.y, vec.z);
}

void Matrix::scale(const float x, const float y, const float z)
{
    float mat[16];
    setIdentityMatrix(mat);
    mat[0] = x;
    mat[5] = y;
    mat[10] = z;

    multMatrix(mat);
}

void Matrix::scale(const fl3 &vec)
{
    scale(vec.x, vec.y, vec.z);
}

void Matrix::rotate(const float degrees, const float x, const float y, const float z)
{
    float mat[16];
    const float radians = DEGTORAD(degrees);
    const float co = cos(radians);
    const float si = sin(radians);
    const float x2 = x*x;
    const float y2 = y*y;
    const float z2 = z*z;

    mat[0] = x2 + (y2 + z2) * co;
    mat[4] = x * y * (1 - co) - z * si;
    mat[8] = x * z * (1 - co) + y * si;
    mat[12]= 0.0f;

    mat[1] = x * y * (1 - co) + z * si;
    mat[5] = y2 + (x2 + z2) * co;
    mat[9] = y * z * (1 - co) - x * si;
    mat[13]= 0.0f;

    mat[2] = x * z * (1 - co) - y * si;
    mat[6] = y * z * (1 - co) + x * si;
    mat[10]= z2 + (x2 + y2) * co;
    mat[14]= 0.0f;

    mat[3] = 0.0f;
    mat[7] = 0.0f;
    mat[11]= 0.0f;
    mat[15]= 1.0f;

    multMatrix(mat);
}

void Matrix::rotate(const float degrees, const fl3 &axis)
{
    rotate(degrees, axis.x, axis.y, axis.z);
}

void Matrix::lookAtHelper(const fl3 &pos, const fl3 &dir, const fl3 &up, const fl3 &right)
{
    entries_[0] = right.x;
    entries_[4] = right.y;
    entries_[8] = right.z;
    entries_[12] = 0.0f;

    entries_[1] = up.x;
    entries_[5] = up.y;
    entries_[9] = up.z;
    entries_[13] = 0.0f;

    entries_[2] = -dir.x;
    entries_[6] = -dir.y;
    entries_[10] = -dir.z;
    entries_[14] = 0.0f;

    entries_[3] = 0.0f;
    entries_[7] = 0.0f;
    entries_[11] = 0.0f;
    entries_[15] = 1.0f;

    translate(-pos);
}

void Matrix::lookAt(const float xPos, const float yPos, const float zPos,
                    const float xLook, const float yLook, const float zLook,
                    const float xUp, const float yUp, const float zUp)
{
    fl3 dir (xLook - xPos, yLook - yPos, zLook - zPos);
    dir.normalize();

    fl3 up (xUp, yUp, zUp);

    fl3 right = fl3::cross(dir, up);
    right.normalize();

    up = fl3::cross(right, dir);
    up.normalize();

    lookAtHelper(fl3(xPos, yPos, zPos), dir, up, right);
}

void Matrix::lookAt(const fl3 &pos, const fl3 &look, const fl3 &up)
{
    fl3 dir = look - pos;
    dir.normalize();
    fl3 right = fl3::cross(dir, up);
    right.normalize();

    fl3 newup = fl3::cross(right, dir);
    newup.normalize();

    lookAtHelper(pos, dir, newup, right);
}

void Matrix::perspective(const float fov, const float ratio, const float nearp, const float farp)
{
    const float f = 1.0f / tan(fov * (M_PI / 360.f));

    setIdentityMatrix(entries_);

    entries_[0] = f / ratio;
    entries_[5] = f;
    entries_[10] = (farp + nearp) / (nearp - farp);
    entries_[14] = (2.0f * farp * nearp) / (nearp - farp);
    entries_[11] = -1.0f;
    entries_[15] = 0.0f;
}

void Matrix::ortho(const float left, const float right, const float bottom, const float top, const float nearp, const float farp)
{
    setIdentityMatrix(entries_);
    entries_[0] = 2 / (right - left);
    entries_[5] = 2 / (top - bottom);
    entries_[10] = -2 / (farp - nearp);
    entries_[12] = -(right + left) / (right - left);
    entries_[13] = -(top + bottom) / (top - bottom);
    entries_[14] = -(farp + nearp) / (farp - nearp);
}

void Matrix::frustum(const float left, const float right, const float bottom, const float top, const float nearp, const float farp)
{
    setIdentityMatrix(entries_);
    entries_[0] = 2 * nearp / (right - left);
    entries_[5] = 2 * nearp / (top - bottom);
    entries_[8] = (right + left) / (right - left);
    entries_[9] = (top + bottom) / (top - bottom);
    entries_[10] = (farp + nearp) / (farp - nearp);
    entries_[11] = -1.0f;
    entries_[12] = 2 * farp * nearp / (farp - nearp);
    entries_[15] = 0.0f;
}

const float* Matrix::data() const
{
    return entries_;
}

void Matrix::print()
{
    for (int i = 0; i < 4; i ++) {
        printf("%f\t%f\t%f\t%f\n", entries_[i], entries_[i+4], entries_[i+8], entries_[i+12]); fflush(stdout);
    }
}
