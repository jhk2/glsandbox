#ifndef SHADER_H
#define SHADER_H

#ifdef _WIN32
#include <windows.h>
#endif
#include <gl/gl.h>
#include <gl/glext.h>

/*
*	Represents a whole shader program (with any number of shader stages implemented)
*	Compilers, links, and runs various types of shaders
*
*	Note: We will define a shader program as the larger entity containing all of the shader stages
*	that all operate in sequence when the program is bound (using glUseProgram).  A single shader
*	refers to the program which represents a single stage in the larger shader program.
*	Thus, multiple shaders will be compiled individually and linked together into a single shader program.
*	However, for convenience, the class representing the whole shader program is just called "Shader",
*	and individual shaders are not visible to the user of this class.
*
*	All shaders for a particular shader program are loaded from a single source file
*	The source file should use #ifdef/#endif pair to denote which source code to use for
*	which shader stage
*	Identifiers for each shader stage:
*	_VERTEX_, _FRAGMENT_, _GEOMETRY_, _TESSCONTROL_, _TESSEVAL_, _COMPUTE_
*/

class Shader
{
	public:
		// enumeration of individual shader stage types
		enum SHADER_TYPES {
			VERTEX_SHADER = 0x01,
			FRAGMENT_SHADER = 0x02,
			GEOMETRY_SHADER = 0x04,
			TESSELLATION_SHADER = 0x08,
            COMPUTE_SHADER = 0x10
		};
		
		virtual void use() const = 0;
		GLuint getProgramID() const { return program_id_; }

		GLint getUniformLocation(const GLchar *name) const;
		GLuint getUniformBlockIndex(const GLchar *blockName) const;
		virtual void validate() const = 0;
	protected:
		// loads shader of specific type from the source file
		void loadShader(const char *sourceFile, const GLenum type);
		// prints the compiler log for a single shader
		static void printShaderLog(const GLuint id);
		GLuint program_id_;
};

class ShaderProgram : public Shader
{
	public:		
		ShaderProgram(const char *filename, const unsigned int stages);
		virtual ~ShaderProgram();
		
		void use() const;
		void validate() const;
	private:
		// loads the whole shader program (by loading each of the individual shaders)
		void loadShaderProgram(const char *sourceFile, const unsigned int stages);
		
};

// class for individual shader pipeline objects
class ShaderPipeline : public Shader
{
	public:
		ShaderPipeline(const char *filename, const unsigned int stages);
		virtual ~ShaderPipeline();
	
		void use() const;
		void validate() const;
	private:
		void loadShaderPipeline(const char *sourceFile, const unsigned int stages);
};

#endif // SHADER_H
