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
        void toMatrixAll(MatrixStack &mstack) const;
        void toMatrixPj(MatrixStack &mstack) const;
        void toMatrixPj(Matrix &matrix) const;
        virtual void toMatrixMv(MatrixStack &mstack) const = 0;
        virtual void toMatrixMv(Matrix &matrix) const = 0;

		// means different things in first vs third person
		virtual Camera& move(fl3 &tomove) = 0;
		// rotate the camera by degrees
		Camera& rotate(fl2 &torot);
		// setting position, rotation
		Camera& setRot(fl2 &newrot);
		Camera& setPos(fl3 &newpos);
        fl3 getPos() const { return pos_; }
	protected:
		fl2 rot_; // around x and y axes (pitch and yaw) by degrees (no roll)
		fl3 pos_;
		double fovy_, aspect_, zNear_, zFar_;
};

class FirstPersonCamera : public Camera
{
	public:
		FirstPersonCamera();
		FirstPersonCamera(double fovy, double aspect, double zNear, double zFar);
		virtual ~FirstPersonCamera();
        void toMatrixMv(MatrixStack &mstack) const;
        void toMatrixMv(Matrix &matrix) const;
		// moving the camera in camera space, except for Y which is always absolute
		// positive x is right, positive y is up, positive z is backward
		Camera& move(fl3 &tomove);
};

class ThirdPersonCamera : public Camera
{
	public:
		ThirdPersonCamera();
		ThirdPersonCamera(double fovy, double aspect, double zNear, double zFar, double distance);
		virtual ~ThirdPersonCamera();
        void toMatrixMv(MatrixStack &mstack) const;
        void toMatrixMv(Matrix &matrix) const;
		// partial transformation for drawing the player avatar
        void toMatrixMvAvatar(MatrixStack &mstack) const;
        void toMatrixMvAvatar(Matrix &matrix) const;
		// movement in world space, not camera space, or just the point the camera is looking at
		// it is the responsibility of the caller to figure out how the point moves
		Camera& move(fl3 &tomove);
		ThirdPersonCamera& setDistance(double distance);
	private:
		double distance_;
};
#endif // CAMERA_H
