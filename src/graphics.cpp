#include <fstream>
#include <stdexcept>
#include <array>

#include <fangpp/graphics.hpp>

GLFWwindow *Graphics::initGL()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    glfwSetErrorCallback(glfwError);
    // Minimum required version is 3.0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SAMPLES, 8);  // 8 x multi-sampling
    
    // Create window
    GLFWwindow *window = glfwCreateWindow(defaultWidth, defaultHeight, "Fang", nullptr, nullptr);
    if (!window)
    {
        throw std::runtime_error("Failed to create window");
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);  // allow modifying object from callback
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);
    
    // Initialize GLEW
    const GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        throw std::runtime_error("Failed to initialize GLEW: " +
            std::string(reinterpret_cast<const char *>(glewGetErrorString(err))));
    }
    glfwSwapInterval(1);  // #frames until buffer swap
    // Setup basic OpenGL options
    CHKERRGL(glClearColor(0.8, 0.8, 0.8, 1.0));
    CHKERRGL(glEnable(GL_MULTISAMPLE));
    //CHKERRGL(glEnable(GL_CULL_FACE));
    CHKERRGL(glEnable(GL_BLEND));
    CHKERRGL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    
    return window;
}

Graphics::Graphics() : 
    window(initGL()), 
    circles(), 
    lines(), 
    text("fonts/LiberationMono-Regular.ttf")
{
    // Initialize VAOs and associated VBOs, as well as shader program
    updateProjection(defaultWidth, defaultHeight);
}

void Graphics::run()
{
    sound.play(Sound::MAIN_THEME);  // start playing main theme on loop
    
    while (!glfwWindowShouldClose(window))
    {
        // Update sound system
        sound.update();
        
        GLint width, height;
        glfwGetFramebufferSize(window, &width, &height);
        // Compute current aspect ratio
        const GLfloat aspect = static_cast<GLfloat>(width) / height;
        
        CHKERRGL(glViewport(0, 0, width, height));
        CHKERRGL(glClear(GL_COLOR_BUFFER_BIT));
        
        // Update projection matrix (uniform)
        if (framebufferResized)
        {
            updateProjection(width, height);
            framebufferResized = false;
        }
        
        // Draw lines associated with edges
        lines.draw();
        // Draw circles representing nodes
        circles.draw();
        text.drawAtCentered(L"Hellö", 0.25f*width, 0.25f*height, glm::vec3(0.0f, 0.0f, 0.0f), Text::CENTER_BOTH, aspect, 0.5);
        text.drawAtCentered(L"2", -0.25f*width, 0.25f*height, glm::vec3(0.0f, 0.0f, 0.0f), Text::CENTER_BOTH, aspect);
        text.drawAtCentered(L"3", -0.25f*width, -0.25f*height, glm::vec3(0.0f, 0.0f, 0.0f), Text::CENTER_BOTH, aspect);
        text.drawAtCentered(L"4", 0.25f*width, -0.25f*height, glm::vec3(0.0f, 0.0f, 0.0f), Text::CENTER_BOTH, aspect);
        text.drawAtCentered(L"This-is-a-test", -0.5f*width, 0.0f, glm::vec3(0.0f, 0.0f, 0.0f), Text::CENTER_VERTICAL);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

Graphics::~Graphics()
{
    // Free OpenGL resources
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Graphics::updateProjection(const GLfloat width, const GLfloat height) const
{
    // Note: Also update other projections
    circles.updateProjection(width, height);
    lines.updateProjection(width, height);
    text.updateProjection(width, height);
}

void Graphics::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    (void)width;   // unused
    (void)height;  // unused
    
    auto graphics = reinterpret_cast<Graphics *>(glfwGetWindowUserPointer(window));
    graphics->framebufferResized = true;
}
    
void Graphics::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    (void)mods;  // unused
    
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Left mouse button was pressed
        // Compute device coordinates from cursor pos
        GLdouble xpos = 0.0, ypos = 0.0;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        GLint width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);        
        // Map from screen coordinates to device coordinates
        xpos = 2.0 * xpos / width - 1.0;
        ypos = 1.0 - 2.0 * ypos / height;  // flip y-axis
        // Inverse orthographic projection (multiply by aspect ratio)
        xpos *= static_cast<GLdouble>(width) / height;
        
        // TODO: Check all circle offset positions individually
        const GLdouble cx = 0.5, cy = 0.5;  // top right (blue) circle
        const GLdouble dx = xpos - cx;
        const GLdouble dy = ypos - cy;
        
        if (dx*dx + dy*dy <= 0.25*0.25)
        {
            auto graphics = reinterpret_cast<Graphics *>(glfwGetWindowUserPointer(window));
            graphics->sound.play(Sound::CLICK_SFX);
        }
    }
}

void Graphics::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)scancode;  // unused
    (void)mods;      // unused
    
    if ((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void Graphics::glfwError(GLint error, const GLchar *description)
{
    (void)error;  // unused
    
    throw std::runtime_error("GLFW error: " + std::string(description));
}
