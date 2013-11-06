#include "matrixstack.h"
#include <cmath>
#include "glextensionfuncs.h"
#include "constants.h"
#include <stdio.h>

// MatrixStack constructor
MatrixStack::MatrixStack():
	    mInit(false)
{
}

// MatrixStack destructor
MatrixStack::~MatrixStack()
{
}

void MatrixStack::copy(const MatrixTypes type, Matrix &dest) const
{
    dest.loadMatrix(mMatrix[type].data());
}

// send the uniform locations to MatrixStack
void
MatrixStack::initUniformLocs(const GLuint modelviewLoc, const GLuint projLoc)
{
    mInit = true;
    mUniformLoc[MODELVIEW] = modelviewLoc;
    mUniformLoc[PROJECTION] = projLoc;
}

// glPushMatrix implementation
void
MatrixStack::pushMatrix(const MatrixTypes aType) {
    Matrix *aux = new Matrix(mMatrix[aType]);
    mMatrixStack[aType].push_back(aux);
}

// glPopMatrix implementation
void
MatrixStack::popMatrix(const MatrixTypes aType) {

    Matrix *m = mMatrixStack[aType][mMatrixStack[aType].size()-1];
    mMatrix[aType].loadMatrix(m->data());
    mMatrixStack[aType].pop_back();
    delete m;
}

// glLoadIdentity implementation
void
MatrixStack::loadIdentity(const MatrixTypes aType)
{
    mMatrix[aType].loadIdentity();
}

// glMultMatrix implementation
void
MatrixStack::multMatrix(const MatrixTypes aType, const float *aMatrix)
{
    mMatrix[aType].multMatrix(aMatrix);
}

// glLoadMatrix implementation
void
MatrixStack::loadMatrix(const MatrixTypes aType, const float *aMatrix)
{
    mMatrix[aType].loadMatrix(aMatrix);
}

// glTranslate implementation with matrix selection
void
MatrixStack::translate(const MatrixTypes aType, const float x, const float y, const float z)
{
    mMatrix[aType].translate(x, y, z);
}

// glTranslate on the MODELVIEW matrix
void
MatrixStack::translate(const float x, const float y, const float z)
{
    translate(MODELVIEW, x,y,z);
}

void MatrixStack::translate(const fl3 &vec)
{
    translate(MODELVIEW, vec.x, vec.y, vec.z);
}

// glScale implementation with matrix selection
void
MatrixStack::scale(const MatrixTypes aType, const float x, const float y, const float z)
{
    mMatrix[aType].scale(x, y, z);
}

// glScale on the MODELVIEW matrix
void
MatrixStack::scale(const float x, const float y, const float z)
{
    scale(MODELVIEW, x, y, z);
}

// glRotate implementation with matrix selection
void
MatrixStack::rotate(const MatrixTypes aType, const float angle, const float x, const float y, const float z)
{
    mMatrix[aType].rotate(angle, x, y, z);
}

// glRotate implementation in the MODELVIEW matrix
void
MatrixStack::rotate(const float angle, const float x, const float y, const float z)
{
    rotate(MODELVIEW,angle,x,y,z);
}

// gluLookAt implementation
void
MatrixStack::lookAt(const float xPos, const float yPos, const float zPos,
                    const float xLook, const float yLook, const float zLook,
                    const float xUp, const float yUp, const float zUp)
{
    mMatrix[MODELVIEW].lookAt(xPos, yPos, zPos,
                              xLook, yLook, zLook,
                              xUp, yUp, zUp);
}

// gluPerspective implementation
void
MatrixStack::perspective(const float fov, const float ratio, const float nearp, const float farp)
{
    mMatrix[PROJECTION].perspective(fov, ratio, nearp, farp);
}

// glOrtho implementation
void
MatrixStack::ortho(const float left, const float right, const float bottom, const float top, const float nearp, const float farp)
{
    mMatrix[PROJECTION].ortho(left, right, bottom, top, nearp, farp);
}

// glFrustum implementation
void
MatrixStack::frustum(const float left, const float right, const float bottom, const float top, const float nearp, const float farp)
{
    mMatrix[PROJECTION].frustum(left, right, bottom, top, nearp, farp);
}

/* -----------------------------------------------------
		     SEND MATRICES TO OPENGL
------------------------------------------------------*/

// to be used with uniform variables
void
MatrixStack::matrixToUniform(const MatrixTypes aType)
{
    if (mInit) {
        glUniformMatrix4fv(mUniformLoc[aType], 1, false, mMatrix[aType].data());
    }
}

