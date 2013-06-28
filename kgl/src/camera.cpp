#include "camera.h"
#include "constants.h"
#include <math.h>

Camera::Camera() : rot_(), pos_(), fovy_(0), aspect_(0), zNear_(0), zFar_(0)
{
	
}

Camera::Camera(double fovy, double aspect, double zNear, double zFar)
{
	init(fovy, aspect, zNear, zFar);
}

Camera::~Camera()
{
	
}

void Camera::init(double fovy, double aspect, double zNear, double zFar)
{
	fovy_ = fovy;
	aspect_ = aspect;
	zNear_ = zNear;
	zFar_ = zFar;
}

void Camera::toMatrixAll(MatrixStack &mstack) const
{
	toMatrixPj(mstack);
	toMatrixMv(mstack);
}

void Camera::toMatrixPj(MatrixStack &mstack) const
{
	/*
	double xmin, xmax, ymin, ymax;
	ymax = zNear_ * tan(fovy_ * M_PI / 360.0);
	ymin = -ymax;
	xmin = ymin * aspect_;
	xmax = ymax * aspect_;
	mstack.loadIdentity(MatrixStack::PROJECTION);
	mstack.frustum(xmin, xmax, ymin, ymax, zNear_, zFar_);
	*/
	mstack.perspective(fovy_, aspect_, zNear_, zFar_);
}

void Camera::toMatrixPj(Matrix &matrix) const
{
    matrix.perspective(fovy_, aspect_, zNear_, zFar_);
}

Camera& Camera::rotate(fl2 &torot)
{
	rot_ += torot;
	// limit pitch to -90 to 90
	rot_.x = min(90, max(-90, rot_.x));
	// limit left/right to -180 to 180 to prevent overflow
	if (rot_.y > 180) {
		rot_.y = rot_.y - 360;
	} else if (rot_.y < -180) {
		rot_.y = rot_.y + 360;
	}
	return *this;
}

Camera& Camera::setRot(fl2 &newrot)
{
	rot_ = newrot;
	return *this;
}

Camera& Camera::setPos(fl3 &newpos)
{
	pos_ = newpos;
	return *this;
}

FirstPersonCamera::FirstPersonCamera() : Camera() {}
	
FirstPersonCamera::FirstPersonCamera(double fovy, double aspect, double zNear, double zFar) :
	Camera(fovy, aspect, zNear, zFar) {}

FirstPersonCamera::~FirstPersonCamera() {}

Camera& FirstPersonCamera::move(fl3 &tomove)
{
	float r = tomove.z * cos(DEGTORAD(rot_.x));
	pos_.x += r * sin(DEGTORAD(rot_.y)) + tomove.x * cos(DEGTORAD(rot_.y));
	pos_.z += r * cos(DEGTORAD(rot_.y)) - tomove.x * sin(DEGTORAD(rot_.y));
	pos_.y += tomove.y - tomove.z * sin(DEGTORAD(rot_.x));
	return *this;
}

void FirstPersonCamera::toMatrixMv(MatrixStack &mstack) const
{
	mstack.loadIdentity(MatrixStack::MODELVIEW);
	mstack.rotate(-rot_.x, 1, 0, 0);
	mstack.rotate(-rot_.y, 0, 1, 0);
	mstack.translate(-pos_.x, -pos_.y, -pos_.z);
}

void FirstPersonCamera::toMatrixMv(Matrix &matrix) const
{
    matrix.loadIdentity();
    matrix.rotate(-rot_.x, 1, 0, 0);
    matrix.rotate(-rot_.y, 0, 1, 0);
    matrix.translate(-pos_.x, -pos_.y, -pos_.z);
}

ThirdPersonCamera::ThirdPersonCamera() : Camera(), distance_(0) {}

ThirdPersonCamera::ThirdPersonCamera(double fovy, double aspect, double zNear, double zFar, double distance) :
	Camera(fovy, aspect, zNear, zFar), distance_(distance) {}

ThirdPersonCamera::~ThirdPersonCamera() {}

Camera& ThirdPersonCamera::move(fl3 &tomove)
{
	float r = tomove.z * cos(DEGTORAD(rot_.x));
	pos_.x += r * sin(DEGTORAD(rot_.y)) + tomove.x * cos(DEGTORAD(rot_.y));
	pos_.z += r * cos(DEGTORAD(rot_.y)) - tomove.x * sin(DEGTORAD(rot_.y));
	pos_.y += tomove.y;// - tomove.z * sin(DEGTORAD(rot_.x));
	return *this;
}

void ThirdPersonCamera::toMatrixMv(MatrixStack &mstack) const
{
	mstack.loadIdentity(MatrixStack::MODELVIEW);
	mstack.translate(0, 0, -distance_);
	mstack.rotate(-rot_.x, 1, 0, 0);
	mstack.rotate(-rot_.y, 0, 1, 0);
	mstack.translate(-pos_.x, -pos_.y, -pos_.z);
}

void ThirdPersonCamera::toMatrixMv(Matrix &matrix) const
{
    matrix.loadIdentity();
    matrix.translate(0, 0, -distance_);
    matrix.rotate(-rot_.x, 1, 0, 0);
    matrix.rotate(-rot_.y, 0, 1, 0);
    matrix.translate(-pos_.x, -pos_.y, -pos_.z);
}

void ThirdPersonCamera::toMatrixMvAvatar(MatrixStack &mstack) const
{
	mstack.loadIdentity(MatrixStack::MODELVIEW);
	mstack.translate(0, 0, -distance_);
	mstack.rotate(-rot_.x, 1, 0, 0);
	//~ mstack.rotate(-rot_.y, 0, 1, 0);
}

void ThirdPersonCamera::toMatrixMvAvatar(Matrix &matrix) const
{
    matrix.loadIdentity();
    matrix.translate(0, 0, -distance_);
    matrix.rotate(-rot_.x, 1, 0, 0);
}

ThirdPersonCamera& ThirdPersonCamera::setDistance(double distance)
{
	distance_ = distance;
	return *this;
}
