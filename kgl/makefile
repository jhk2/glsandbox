# source files for library
LIBSRC = test.cpp
# library name
LIBNAME = libkgl.lib
# compiler with flags
CC = cl /EHsc /nologo /c
# linker with flags
LNK = lib /nologo
# output obj files
OBJ = winglbase.obj glextensionfuncs.obj matrixstack.obj shader.obj mesh.obj

# always doing a full rebuild for now
# since I don't know how to do autodependency for nmake
all: clean kgl

.cpp.obj:
	$(CC) $<

kgl: $(OBJ)
	$(LNK) /out:$(LIBNAME) $(OBJ)
	
clean:
	del *.lib *.obj