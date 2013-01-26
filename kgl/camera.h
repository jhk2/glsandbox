#ifndef CAMERA_H
#define CAMERA_H
#include "matrixstack.h"
#include "utils.h"
class Camera
{
	public:
		Camera();
		Camera(double fovy, double aspect, double zNear, double zFar);
		virtual ~Camera();
		void init(double fovy, double aspect, double zNear, double zFar);
		// send the current camera parameters to the current matrix
		void toMatrixAll(MatrixStack &mstack);
		void toMatrixPj(MatrixStack &mstack);
		void toMatrixMv(MatrixStack &mstack);
		// moving the camera in camera space, except for Y which is always absolute
		// positive x is right, positive y is up, positive z is backward
		Camera& move(fl3 &tomove);
		// rotate the camera by degrees
		Camera& rotate(fl2 &torot);
		// setting position, rotation
		Camera& setRot(fl2 &newrot);
		Camera& setPos(fl3 &newpos);
	private:
		fl2 rot_; // around x and y axes (pitch and yaw) by degrees (no roll)
		fl3 pos_;
		double fovy_, aspect_, zNear_, zFar_;
};
#endif // CAMERA_H