#include <stdexcept>
#include <fstream>

#include <fangpp/gl_common.hpp>

std::string getGLErrorString(GLenum error)
{
    switch (error) {
        case GL_NO_ERROR:            return "No error";
        case GL_INVALID_ENUM:        return "Invalid enum";
        case GL_INVALID_VALUE:       return "Invalid value";
        case GL_INVALID_OPERATION:   return "Invalid operation";
        case GL_STACK_OVERFLOW:      return "Stack overflow";
        case GL_STACK_UNDERFLOW:     return "Stack underflow";
        case GL_OUT_OF_MEMORY:       return "Out of memory";
        case GL_TABLE_TOO_LARGE:     return "Table too large";
        default:                     return "Unknown GL error";
    }
}

GLuint createGLShader(const GLenum shaderType, const char *filename)
{
    std::ifstream file (filename);
    if (!file)
    {
        throw std::runtime_error("Failed to open shader code file");
    }
    // Determine file size and reset file pointer to beginning
    file.seekg(0, file.end);
    int size = file.tellg();
    file.seekg(0, file.beg);
    // Read shader code into buffer
    GLchar *shaderSource = new GLchar[size];
    file.read(shaderSource, size);
    file.close();
    
    GLuint shader;
    CHKERRGL(shader = glCreateShader(shaderType));
    CHKERRGL(glShaderSource(shader, 1, &shaderSource, &size));
    // Cleanup
    delete[] shaderSource;  // Note: glShaderSource copies source code
    
    CHKERRGL(glCompileShader(shader));
    // Check if shader was compiled successfully
    GLint success;
    CHKERRGL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    if (!success)
    {
        GLint maxInfo = 0;
        CHKERRGL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxInfo));
        
        GLchar *infoLog = new GLchar[maxInfo];
        CHKERRGL(glGetShaderInfoLog(shader, maxInfo, nullptr, infoLog));
        const std::string infoLogString (infoLog);  // convert to string
        // Cleanup
        CHKERRGL(glDeleteShader(shader));
        delete[] infoLog;
        
        throw std::runtime_error("Failed to compile shader: " + infoLogString);
    }
    
    return shader;
}

GLuint createGLShaderProgram(const char *vsFile, const char *fsFile)
{
    const GLuint vShader = createGLShader(GL_VERTEX_SHADER, vsFile);
    const GLuint fShader = createGLShader(GL_FRAGMENT_SHADER, fsFile);
    
    GLuint shaderProgram;
    CHKERRGL(shaderProgram = glCreateProgram());
    // Attach shaders to program
    CHKERRGL(glAttachShader(shaderProgram, vShader));
    CHKERRGL(glAttachShader(shaderProgram, fShader));
    // Link shader program
    CHKERRGL(glLinkProgram(shaderProgram));
    // Check for success
    GLint success;
    CHKERRGL(glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success));
    if (!success)
    {
        GLint maxInfo = 0;
        CHKERRGL(glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxInfo));
        
        GLchar *infoLog = new GLchar[maxInfo];
        CHKERRGL(glGetProgramInfoLog(shaderProgram, maxInfo, nullptr, infoLog));
        const std::string infoLogString (infoLog);
        // Cleanup
        CHKERRGL(glDetachShader(shaderProgram, vShader));
        CHKERRGL(glDetachShader(shaderProgram, fShader));
        CHKERRGL(glDeleteShader(vShader));
        CHKERRGL(glDeleteShader(fShader));
        CHKERRGL(glDeleteProgram(shaderProgram));
        delete[] infoLog;
        
        throw std::runtime_error("Failed to link shader program: " + infoLogString);
    }
    
    // Cleanup: Shaders are not needed anymore after linking 
    CHKERRGL(glDetachShader(shaderProgram, vShader));
    CHKERRGL(glDetachShader(shaderProgram, fShader));
    CHKERRGL(glDeleteShader(vShader));
    CHKERRGL(glDeleteShader(fShader));
    
    return shaderProgram;
}
