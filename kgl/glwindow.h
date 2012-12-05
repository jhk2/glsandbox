#ifndef GLWINDOW_H
#define GLWINDOW_H

class GLWindow
{
	public:
		GLWindow() {};
		virtual ~GLWindow() {};
	protected:
		void initContext() = 0;
		void initFunctions() = 0;
		
};
#endif // GLWINDOW_H