# source files for library
LIBSRC = test.cpp
# library name
LIBNAME = libkgl.lib

all: kgl

kgl: test.obj
	lib test.obj

test.obj: test.cpp
	cl test.cpp /c /nologo
	
clean:
	del *.lib *.obj