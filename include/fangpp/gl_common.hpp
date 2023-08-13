#ifndef GL_COMMON_HPP
#define GL_COMMON_HPP

#include "GL/glew.h"
#include "GLFW/glfw3.h"  // automatically includes "GL/gl.h"

#define GLM_FORCE_RADIANS  // always use radians for angles
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // glm::lookAt, glm::rotate, e.t.c.
#include <glm/gtc/matrix_access.hpp>  // column(), row()
#include <glm/gtc/type_ptr.hpp>  // glm::value_ptr

#include <string>

// Convert GLenum error code to respective error text
std::string getGLErrorString(GLenum error);

#define CHKERRGL(call)                                                 \
do                                                                     \
{                                                                      \
    call;                                                              \
    const GLenum err = glGetError();                                   \
    if (err != GL_NO_ERROR) {                                          \
        throw std::runtime_error("GL error in " +                      \
            std::string(__FILE__) + ":" + std::to_string(__LINE__) +   \
            + " " + getGLErrorString(err));                            \
    }                                                                  \
}                                                                      \
while (0)

// Initialize GLFW window, GLEW and global OpenGL settings
void initGL();
// Create shader from type and shader code file path
GLuint createGLShader(const GLenum shaderType, const char *filename);
// Create shader program from file paths of vertex and fragment shader code
GLuint createGLShaderProgram(const char *vsFile, const char *fsFile);

#endif /* GL_COMMON_HPP */
