#ifndef CAMERA_H
#define CAMERA_H
#include "matrixstack.h"
#include "utils.h"
class Camera
{
	public:
		Camera();
        Camera(float fovy, float aspect, float zNear, float zFar);
		virtual ~Camera();
        void init(float fovy, float aspect, float zNear, float zFar);
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
        fl3 getUp() const;
        fl3 getLook() const;
	protected:
		fl2 rot_; // around x and y axes (pitch and yaw) by degrees (no roll)
		fl3 pos_;
        float fovy_, aspect_, zNear_, zFar_;
};

class FirstPersonCamera : public Camera
{
	public:
		FirstPersonCamera();
        FirstPersonCamera(float fovy, float aspect, float zNear, float zFar);
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
        ThirdPersonCamera(float fovy, float aspect, float zNear, float zFar, float distance);
		virtual ~ThirdPersonCamera();
        void toMatrixMv(MatrixStack &mstack) const;
        void toMatrixMv(Matrix &matrix) const;
		// partial transformation for drawing the player avatar
        void toMatrixMvAvatar(MatrixStack &mstack) const;
        void toMatrixMvAvatar(Matrix &matrix) const;
		// movement in world space, not camera space, or just the point the camera is looking at
		// it is the responsibility of the caller to figure out how the point moves
		Camera& move(fl3 &tomove);
        ThirdPersonCamera& setDistance(float distance);
	private:
        float distance_;
};
#endif // CAMERA_H
