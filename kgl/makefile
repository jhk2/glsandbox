# source files for library
LIBSRC = test.cpp
# library name
LIBNAME = libkgl.lib
# compiler with flags
CC = cl /EHsc /nologo /c
# linker with flags
LNK = lib /nologo

all: kgl

kgl: winglbase.obj
	$(LNK) /out:$(LIBNAME) winglbase.obj
	
winglbase.obj: winglbase.cpp
	$(CC) winglbase.cpp
	
clean:
	del *.lib *.obj