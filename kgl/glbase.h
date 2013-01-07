#ifndef GLBASE_H
#define GLBASE_H

/*
*	Class in charge of setting up GL context and handle window functions, like key and mouse input
*/
class GLBase
{
	public:
		GLBase(unsigned int width, unsigned int height) : width_(width), height_(height) {};
		virtual ~GLBase() {};
		virtual void swapBuffers() = 0;
		virtual void update() = 0;
		virtual long long currentMillis() = 0;
			
		unsigned int getWidth() { return width_; }
		unsigned int getHeight() { return height_; }
		bool isActive() { return running_; }
	protected:
		virtual bool initContext() = 0;
		virtual bool initFunctions() = 0;
		
		unsigned int width_, height_;
		bool running_;
};
#endif // GLBASE_H