#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include "gl_common.hpp"
#include "sound.hpp"
#include "circles.hpp"
#include "lines.hpp"
#include "text.hpp"

#include <iostream>
#include <string>
#include <chrono>

class Graphics
{
public:
    Graphics();
    
    void run();
        
    ~Graphics();
private:
    GLFWwindow *initGL();
    
    void updateProjection(const GLfloat width, const GLfloat height) const;
    
    // Callbacks
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void glfwError(GLint error, const GLchar *description);
        
    GLFWwindow *window = nullptr;
    
    Sound sound;
    Circles circles;
    Lines lines;
    Text text;
    
    bool framebufferResized = false;
    
    // Default (initial) resolution of window
    static constexpr const GLint defaultWidth  = 1200;
    static constexpr const GLint defaultHeight = 1000;
};

#endif /* GRAPHICS_HPP */
