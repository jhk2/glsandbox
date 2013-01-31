LIBS =..\libkgl.lib GDI32.lib USER32.lib OPENGL32.lib
INC = ..\..\src
shadow: main.cpp
	cl main.cpp /I $(INC) /MT /EHsc /nologo /Feshadow.exe /link /LTCG $(LIBS)
	
all: shadow

clean:
	del *.exe *.obj