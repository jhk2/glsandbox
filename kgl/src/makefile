# source files for library
LIBSRC = test.cpp
BUILDDIR = ../build
# library name
SINGLELIB = libkglsingle.lib
LIBNAME = libkgl.lib
# compiler with flags
CC = cl /EHsc /nologo /c
# linker with flags
LNK = lib /nologo
# output obj files
OBJ = winglbase.obj glextensionfuncs.obj matrixstack.obj shader.obj camera.obj texture.obj obj.obj
# include directories
INC = include
# additional libraries
LIBS = lib\libpng15.lib lib\zlib.lib lib\libjpeg.lib

# always doing a full rebuild for now
# since I don't know how to do autodependency for nmake
all: clean kgl combine

.cpp.obj:
	$(CC) /I $(INC) $<

kgl: $(OBJ)
	$(LNK) /out:$(BUILDDIR)/$(SINGLELIB) $(OBJ)
	del *.obj
	
combine: kgl
	$(LNK) /LTCG /out:$(BUILDDIR)/$(LIBNAME) $(BUILDDIR)/$(SINGLELIB) $(LIBS) 
	copy ..\build\$(LIBNAME) ..\samples\$(LIBNAME)
clean:
	del ..\build\*.lib