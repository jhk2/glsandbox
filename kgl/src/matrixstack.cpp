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


// send the uniform locations to MatrixStack
void
MatrixStack::initUniformLocs(GLuint modelviewLoc, GLuint projLoc)
{
    mInit = true;
    mUniformLoc[MODELVIEW] = modelviewLoc;
    mUniformLoc[PROJECTION] = projLoc;
}

// glPushMatrix implementation
void
MatrixStack::pushMatrix(MatrixTypes aType) {
    Matrix *aux = new Matrix(mMatrix[aType]);
    mMatrixStack[aType].push_back(aux);
}

// glPopMatrix implementation
void
MatrixStack::popMatrix(MatrixTypes aType) {

    Matrix *m = mMatrixStack[aType][mMatrixStack[aType].size()-1];
    mMatrix[aType].loadMatrix(m->data());
    mMatrixStack[aType].pop_back();
    delete m;
}

// glLoadIdentity implementation
void
MatrixStack::loadIdentity(MatrixTypes aType)
{
    mMatrix[aType].loadIdentity();
}

// glMultMatrix implementation
void
MatrixStack::multMatrix(MatrixTypes aType, const float *aMatrix)
{
    mMatrix[aType].multMatrix(aMatrix);
}

// glLoadMatrix implementation
void
MatrixStack::loadMatrix(MatrixTypes aType, const float *aMatrix)
{
    mMatrix[aType].loadMatrix(aMatrix);
}

// glTranslate implementation with matrix selection
void
MatrixStack::translate(MatrixTypes aType, float x, float y, float z)
{
    mMatrix[aType].translate(x, y, z);
}

// glTranslate on the MODELVIEW matrix
void
MatrixStack::translate(float x, float y, float z)
{
    translate(MODELVIEW, x,y,z);
}

// glScale implementation with matrix selection
void
MatrixStack::scale(MatrixTypes aType, float x, float y, float z)
{
    mMatrix[aType].scale(x, y, z);
}

// glScale on the MODELVIEW matrix
void
MatrixStack::scale(float x, float y, float z)
{
    scale(MODELVIEW, x, y, z);
}

// glRotate implementation with matrix selection
void
MatrixStack::rotate(MatrixTypes aType, float angle, float x, float y, float z)
{
    mMatrix[aType].rotate(angle, x, y, z);
}

// glRotate implementation in the MODELVIEW matrix
void
MatrixStack::rotate(float angle, float x, float y, float z)
{
    rotate(MODELVIEW,angle,x,y,z);
}

// gluLookAt implementation
void
MatrixStack::lookAt(float xPos, float yPos, float zPos,
				    float xLook, float yLook, float zLook,
				    float xUp, float yUp, float zUp)
{
    mMatrix[MODELVIEW].lookAt(xPos, yPos, zPos,
                              xLook, yLook, zLook,
                              xUp, yUp, zUp);
}

// gluPerspective implementation
void
MatrixStack::perspective(float fov, float ratio, float nearp, float farp)
{
    mMatrix[PROJECTION].perspective(fov, ratio, nearp, farp);
}

// glOrtho implementation
void
MatrixStack::ortho(float left, float right, float bottom, float top, float nearp, float farp)
{
    mMatrix[PROJECTION].ortho(left, right, bottom, top, nearp, farp);
}

// glFrustum implementation
void
MatrixStack::frustum(float left, float right, float bottom, float top, float nearp, float farp)
{
    mMatrix[PROJECTION].frustum(left, right, bottom, top, nearp, farp);
}

/* -----------------------------------------------------
		     SEND MATRICES TO OPENGL
------------------------------------------------------*/

// to be used with uniform variables
void
MatrixStack::matrixToUniform(MatrixTypes aType)
{
    if (mInit) {
        glUniformMatrix4fv(mUniformLoc[aType], 1, false, mMatrix[aType].data());
    }
}

