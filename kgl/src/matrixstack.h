#ifndef MATRIXSTACK_H
#define MATRIXSTACK_H

#include <vector>
#include <stdlib.h>
#include <string.h>
#include "matrix.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glext.h>

// this is just a slightly modified version of Lighthouse3D's VSML library for simulating 
// the GL matrix stack for newer versions of OpenGL
// http://www.lighthouse3d.com/very-simple-libs

class MatrixStack {

	public:

		// enumeration to hold matrix types
		enum MatrixTypes{
            MODELVIEW = 0,
            PROJECTION = 1
		} ;

		MatrixStack();
		// destructor
		virtual ~MatrixStack();

        void copy(const MatrixTypes type, Matrix &dest) const;

		// passing data of uniform locations, or buffer and offsets,
		// to  OpenGL
        void initUniformLocs(const GLuint modelviewLoc, const GLuint projLoc);

		// translation, rotation and scale
		// a simplified version which affects the MODELVIEW and does
		// not require the matrix type as a parameters
		// and a more complete version so that both PROJECTION and
		// MODELVIEW matrices can be affected
        void translate(const MatrixTypes aType, const float x, const float y, const float z);
        void translate(const float x, const float y, const float z);
        void translate(const fl3 &vec);
        void scale(const MatrixTypes aType, const float x, const float y, const float z);
        void scale(const float x, const float y, const float z);
        void rotate(const MatrixTypes aType, const float angle, const float x, const float y, const float z);
        void rotate(const float angle, const float x, const float y, const float z);

		// multiplying an arbitrary matrix into MODELVIEW or PROJECTION
        void multMatrix(const MatrixTypes aType, const float *aMatrix);

		// Loading specific matrices
        void loadIdentity(const MatrixTypes aType);
        void loadMatrix(const MatrixTypes aType, const float *aMatrix);

		// push and pop functionality. There is a stack for each
		// matrix type
        void pushMatrix(const MatrixTypes aType);
        void popMatrix(const MatrixTypes aType);

		// gluLookAt implementation. Works on the MODELVIEW matrix
        void lookAt(const float xPos, const float yPos, const float zPos,
                    const float xLook, const float yLook, const float zLook,
                    const float xUp, const float yUp, const float zUp);

		// gluPerspective implementation. Works on the PROJECTION matrix
        void perspective(const float fov, const float ratio, const float nearp, const float farp);

		// glOrtho implementation. Works on the PROJECTION matrix
        void ortho(const float left, const float right, const float bottom, const float top, const float nearp=-1.0f, const float farp=1.0f);

		// glFrustum implementation. Works on the PROJECTION matrix
        void frustum(const float left, const float right, const float bottom, const float top, const float nearp, const float farp);

		// send matrices to OpenGL
        void matrixToUniform(const MatrixTypes aType);

	protected:

		bool mInit;

		// the matrix stacks, one for each matrix stack
        std::vector<Matrix*> mMatrixStack[2];

        // the two matrices (at the top of stack)
        Matrix mMatrix[2];

		// the uniform locations
		GLuint mUniformLoc[2];

		// buffer and offsets for uniform blocks
		GLuint mBuffer, mOffset[2];

};
#endif
