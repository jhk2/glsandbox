LIBS =..\kgl\libkgl.lib GDI32.lib USER32.lib OPENGL32.lib
shadow: main.cpp
	cl main.cpp /MT /EHsc /nologo /Feshadow.exe /link /LTCG $(LIBS)
	
all: shadow

clean:
	del *.exe *.obj