#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include "gl_common.hpp"
#include "sound.hpp"
#include "circles.hpp"
#include "lines.hpp"
#include "text.hpp"
#include "game_state.hpp"

#include <iostream>
#include <string>
#include <chrono>

// TODO: Rename Graphics to something like GameApplication etc.
class Graphics
{
public:
    Graphics();
    
    void run();
        
    ~Graphics();
private:
    GLFWwindow *initGL();
    
    void updateProjection(const GLfloat width, const GLfloat height) const;
    
    // Play appropriate sound for the status of the game after a move
    void playStatusSound(Game::Status status, bool wasUserPlayingAsBoeg);
    
    // Return index of vertex that was clicked by the user if any
    uint32_t getClickedVertexByIndex() const;
    
    // Callbacks
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void glfwError(GLint error, const GLchar *description);
    
    GLFWwindow *window = nullptr;
    
    Game gameState;
    Sound sound;
    Circles circles;
    Lines lines;
    Text text;
    
    bool framebufferResized = false;
    
    // Default (initial) resolution of window
    static constexpr const GLint defaultWidth  = 1200;
    static constexpr const GLint defaultHeight = 1000;
    static constexpr const uint32_t nPlayableCharacters = 7;
    
    // Colors of different playable characters
    static constexpr const glm::vec3 playerColors[nPlayableCharacters] =
    {
        {173.f/255.f, 6.f/255.f, 6.f/255.f},    // red
        {27.f/255.f, 137.f/255.f, 25.f/255.f},  // green
        {25.f/255.f, 26.f/255.f, 177.f/255.f},  // blue
        {223.f/255.f, 224.f/255.f, 38.f/255.f}, // yellow
        {184.f/255.f, 95.f/255.f, 10.f/255.f},  // orange
        {97.f/255.f, 24.f/255.f, 184.f/255.f},  // purple
        {0.9f, 0.9f, 0.9f}                      // white
    };
};

#endif /* GRAPHICS_HPP */
