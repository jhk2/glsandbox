#ifndef MATRIXSTACK_H
#define MATRIXSTACK_H

#include <vector>
#include <stdlib.h>
#include <string.h>

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
			MODELVIEW,
			PROJECTION
		} ;

		/* not using the singleton for now
		// the single instance of MatrixStack
		static MatrixStack* gInstance;

		// getting the pointer to the instance
		static MatrixStack* instance (void);
		*/

		MatrixStack();
		// destructor
		virtual ~MatrixStack();

		// passing data of uniform locations, or buffer and offsets,
		// to  OpenGL
		void initUniformLocs(GLuint modelviewLoc, GLuint projLoc);

		// translation, rotation and scale
		// a simplified version which affects the MODELVIEW and does
		// not require the matrix type as a parameters
		// and a more complete version so that both PROJECTION and
		// MODELVIEW matrices can be affected
		void translate(MatrixTypes aType, float x, float y, float z);
		void translate(float x, float y, float z);
		void scale(MatrixTypes aType, float x, float y, float z);
		void scale(float x, float y, float z);
		void rotate(MatrixTypes aType, float angle, float x, float y, float z);
		void rotate(float angle, float x, float y, float z);

		// multiplying an arbitrary matrix into MODELVIEW or PROJECTION
		void multMatrix(MatrixTypes aType, float *aMatrix);

		// Loading specific matrices
		void loadIdentity(MatrixTypes aType);
		void loadMatrix(MatrixTypes aType, float *aMatrix);

		// push and pop functionality. There is a stack for each
		// matrix type
		void pushMatrix(MatrixTypes aType);
		void popMatrix(MatrixTypes aType);

		// gluLookAt implementation. Works on the MODELVIEW matrix
		void lookAt(float xPos, float yPos, float zPos,
					float xLook, float yLook, float zLook,
					float xUp, float yUp, float zUp);

		// gluPerspective implementation. Works on the PROJECTION matrix
		void perspective(float fov, float ratio, float nearp, float farp);

		// glOrtho implementation. Works on the PROJECTION matrix
		void ortho(float left, float right, float bottom, float top, float nearp=-1.0f, float farp=1.0f);

		// glFrustum implementation. Works on the PROJECTION matrix
		void frustum(float left, float right, float bottom, float top, float nearp, float farp);

		// returns a pointer to the requestedd matrix
		float *get(MatrixTypes aType);

		// send matrices to OpenGL
		void matrixToUniform(MatrixTypes aType);
		void matrixToGL(MatrixTypes aType);

	protected:

		bool mInit;

		// the matrix stacks, one for each matrix stack
		std::vector<float *> mMatrixStack[2];

		// the two matrices
		float mMatrix[2][16];

		// the uniform locations
		GLuint mUniformLoc[2];

		// buffer and offsets for uniform blocks
		GLuint mBuffer, mOffset[2];

		//create an identity matrix in mat
		void setIdentityMatrix( float *mat, int size=4);

		// vector cross product
		// res = a x b
		void crossProduct( float *a, float *b, float *res);

		//vector normalization
		void normalize(float *a);
};
#endif
