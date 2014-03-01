#include "shader.h"
#include "glextensionfuncs.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include "debug.h"

void Shader::loadShader(const char *sourceFile, const GLenum type)
{
    std::stringstream ss;
    std::ifstream file(sourceFile);
    std::string line;
    if (file.is_open() && file.good())
    {
        getline(file, line);
        // skip past whitespace at the beginning
        while (file.good() && (strcmp(line.c_str(), "\n")==0 || strcmp(line.c_str(), "")==0))
        {
            getline(file, line);
        }
        // #define the shader type, this allows us to write all of the shaders in a single text file, which is convenient
        switch (type)
        {
        case GL_VERTEX_SHADER:
            printf("compiling vertex shader for shader %s\n", sourceFile); fflush(stdout);
            ss << "#define _VERTEX_" << std::endl;
            break;
        case GL_FRAGMENT_SHADER:
            printf("compiling fragment shader for shader %s\n", sourceFile); fflush(stdout);
            ss << "#define _FRAGMENT_" << std::endl;
            break;
        case GL_GEOMETRY_SHADER:
            printf("compiling geometry shader for shader %s\n", sourceFile); fflush(stdout);
            ss << "#define _GEOMETRY_" << std::endl;
            break;
        case GL_TESS_CONTROL_SHADER:
            printf("compiling tessellation control shader for shader %s\n", sourceFile); fflush(stdout);
            ss << "#define _TESSCONTROL_" << std::endl;
            break;
        case GL_TESS_EVALUATION_SHADER:
            printf("compiling tessellation evaluation shader for shader %s\n", sourceFile); fflush(stdout);
            ss << "#define _TESSEVAL_" << std::endl;
            break;
        case GL_COMPUTE_SHADER:
            printf("compiling compute shader for shader %s\n", sourceFile); fflush(stdout);
            ss << "#define _COMPUTE_" << std::endl;
            break;
        default:
            break;
        }
        ss << line << std::endl;
        // read the shader source file contents
        while (file.good())
        {
            getline(file, line);
            ss << line << std::endl;
        }
        file.close();
        // load into gl
        std::string str = ss.str();
        const int length = str.length();
        const char *text = str.c_str();
        //printf("shader text is \n%s\n",text); fflush(stdout);
        // create the individual shader handle
        GLuint id = glCreateShader(type);
        // load the source
        glShaderSource(id, 1, (const char **)&text, &length);
        // compile!
        glCompileShader(id);
        // print any errors from the build if they exist
        printShaderLog(id);
        // attach the shader to the overall shader program
        glAttachShader(program_id_, id);
        // once it is attached to the parent program, you can delete the individual shader handle
        glDeleteShader(id);
    }
}

GLint Shader::getUniformLocation(const GLchar *name) const
{
    return glGetUniformLocation(program_id_, name);
}

GLuint Shader::getUniformBlockIndex(const GLchar *blockName) const
{
    return glGetUniformBlockIndex(program_id_, blockName);
}

/*static*/ void Shader::printShaderLog(const GLuint id)
{
    int infoLogLength = 0;
    char infoLog[1024];
    glGetShaderInfoLog(id, 1024, &infoLogLength, infoLog);
    if (infoLogLength > 0)
        printf("Shader log:\n%s", infoLog); fflush(stdout);
}

/*static*/ void Shader::printProgramLog(const GLuint id)
{
    int infoLogLength = 0;
    char infoLog[1024];
    glGetProgramInfoLog(id, 1024, &infoLogLength, infoLog);
    if (infoLogLength > 0)
        printf("Program log:\n%s", infoLog); fflush(stdout);
}

ShaderProgram::ShaderProgram(const char *filename, const unsigned int stages)
{
    loadShaderProgram(filename, stages);
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(program_id_);
}

void ShaderProgram::use() const
{
    glUseProgram(program_id_);
}

void ShaderProgram::validate() const
{
    glValidateProgram(program_id_);
    printShaderLog(program_id_);
}

void ShaderProgram::loadShaderProgram(const char *sourceFile, const unsigned int stages)
{
    //printf("loading shader program from %s, with stages %x\n", sourceFile, stages);
    program_id_ = glCreateProgram();
    // load each individual shader stage if it was defined
    // you MUST have at least a vertex and fragment shader,
    // or have ONLY a compute shader
    if (stages & VERTEX_SHADER)	loadShader(sourceFile, GL_VERTEX_SHADER);
    if (stages & FRAGMENT_SHADER) loadShader(sourceFile, GL_FRAGMENT_SHADER);
    if (stages & GEOMETRY_SHADER) loadShader(sourceFile, GL_GEOMETRY_SHADER);
    if (stages & TESSELLATION_SHADER) {
        loadShader(sourceFile, GL_TESS_CONTROL_SHADER);
        loadShader(sourceFile, GL_TESS_EVALUATION_SHADER);
    }
    if (stages & COMPUTE_SHADER) loadShader(sourceFile, GL_COMPUTE_SHADER);
    glLinkProgram(program_id_);
    printProgramLog(program_id_);
}

ShaderPipeline::ShaderPipeline(const char *filename, const unsigned int stages)
{
    loadShaderPipeline(filename, stages);
}

ShaderPipeline::~ShaderPipeline()
{
    glDeleteProgramPipelines(1, &program_id_);
}

void ShaderPipeline::use() const
{
    glBindProgramPipeline(program_id_);
}

void ShaderPipeline::validate() const
{
    glValidateProgramPipeline(program_id_);
    printShaderLog(program_id_);
}

void ShaderPipeline::loadShaderPipeline(const char *sourceFile, const unsigned int stages)
{
    glGenProgramPipelines(1, &program_id_);
    glBindProgramPipeline(program_id_);
    if (stages & VERTEX_SHADER) {
        loadShader(sourceFile, GL_VERTEX_SHADER);
    }
    if (stages & FRAGMENT_SHADER) {
        loadShader(sourceFile, GL_FRAGMENT_SHADER);
    }
    if (stages & GEOMETRY_SHADER) {
        loadShader(sourceFile, GL_GEOMETRY_SHADER);
    }
    if (stages & TESSELLATION_SHADER) {
        loadShader(sourceFile, GL_TESS_CONTROL_SHADER);
        loadShader(sourceFile, GL_TESS_EVALUATION_SHADER);
    }
    if(stages & COMPUTE_SHADER) loadShader(sourceFile, GL_COMPUTE_SHADER);
    glLinkProgram(program_id_);
    glBindProgramPipeline(0);
}
