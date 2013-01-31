#ifndef GLBASE_H
#define GLBASE_H

#include "utils.h"
/*
*	Class in charge of setting up GL context and handle window functions, like key and mouse input
*/

enum MOUSE_BUTTON {
	MOUSE_LEFT = 0, MOUSE_MIDDLE = 1, MOUSE_RIGHT = 2
};

class GLBase
{
	public:
		GLBase(unsigned int width, unsigned int height) 
			: width_(width), height_(height) {};
		virtual ~GLBase() {};
		virtual void resize(unsigned int width, unsigned int height) = 0;
		virtual void update() = 0;
		virtual float getdtBetweenUpdates() = 0;
		virtual long double currentMillis() = 0;
		unsigned int getWidth() { return width_; }
		unsigned int getHeight() { return height_; }
		bool isActive() { return running_; }
		virtual void close() = 0;
		
		virtual void loadExtensions() = 0;
		
		// input related calls
		virtual bool isKeyDown(const unsigned int code) = 0;
		virtual bool isKeyPress(const unsigned int code) = 0;
		virtual bool isKeyRelease(const unsigned int code) = 0;
		virtual bool isMouseDown(const MOUSE_BUTTON code) = 0;
		virtual bool isMousePress(const MOUSE_BUTTON code) = 0;
		virtual bool isMouseRelease(const MOUSE_BUTTON code) = 0;
		
		
		// mouse position
		// pixel coordinates
		virtual int2 getMousePixelPos() = 0;
		virtual int2 getMousePixel_dxdy() = 0;
		// normalized coordinates
		virtual fl2 getMouseNormPos() = 0;
		virtual fl2 getMouseNorm_dxdy() = 0;
		
		// mouse cursor hide/show
		virtual void showCursor() = 0;
		virtual void hideCursor() = 0;
		// hold cursor movement (dx,dy will still be calculated)
		virtual void holdCursor(const bool setting) = 0;
		
		void finishFrame() { swapBuffers(); swapIODeviceBuffers(); }
		
	protected:
		virtual void swapBuffers() = 0;
		virtual void swapIODeviceBuffers() = 0;
		virtual bool initContext() = 0;
		virtual bool initFunctions() = 0;
		
		unsigned int width_, height_;
		bool running_;
};

#endif // GLBASE_H