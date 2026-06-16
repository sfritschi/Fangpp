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
    gameState("graphs/graph_fang.graphml", 4, 4),
    circles(gameState.getVertices()), 
    lines(gameState.getLinesFromEdges()), 
    text("fonts/LiberationMono-Regular.ttf")
{
    // Initialize VAOs and associated VBOs, as well as shader program
    updateProjection(defaultWidth, defaultHeight);
}

void Graphics::run()
{
    sound.play(Sound::MAIN_THEME);  // start playing main theme on loop
    
    glfwSetTime(0.0);  // start timer
    
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
        // Note: Need to animate the colors of players at same position
        circles.animateColors(glfwGetTime(), gameState.prepareCharacterPositions());
        circles.draw();
        //text.drawAtCentered(L"Hellö", 0.25f*width, 0.25f*height, glm::vec3(0.0f, 0.0f, 0.0f), Text::CENTER_BOTH, aspect, 0.5);
        //text.drawAtCentered(L"2", -0.25f*width, 0.25f*height, glm::vec3(0.0f, 0.0f, 0.0f), Text::CENTER_BOTH, aspect);
        //text.drawAtCentered(L"3", -0.25f*width, -0.25f*height, glm::vec3(0.0f, 0.0f, 0.0f), Text::CENTER_BOTH, aspect);
        //text.drawAtCentered(L"4", 0.25f*width, -0.25f*height, glm::vec3(0.0f, 0.0f, 0.0f), Text::CENTER_BOTH, aspect);
        //text.drawAtCentered(L"This-is-a-test", -0.5f*width, 0.0f, glm::vec3(0.0f, 0.0f, 0.0f), Text::CENTER_VERTICAL);
        
        
        const auto &userPlayer = gameState.getUserPlayer();
        const auto playerId = gameState.getCurrentPlayer().getId();
        
        if (userPlayer.getId() == playerId)
        {
            const std::wstring diceRollMsg1 = L"You rolled a " + std::to_wstring(gameState.getDiceRoll());
            text.drawAt(diceRollMsg1, -0.5f*width, 0.5f*height - 40.0f, playerColors[playerId], aspect);
        }
        else
        {
            const std::wstring diceRollMsg1 = L"Player " + std::to_wstring(playerId + 1) + L" rolled a " + std::to_wstring(gameState.getDiceRoll());
            text.drawAt(diceRollMsg1, -0.5f*width, 0.5f*height - 40.0f, playerColors[playerId], aspect);
        }
        
        // Draw name of the location where the user is currently
        uint32_t vertexId = userPlayer.getPosition();
        if (userPlayer.getId() == gameState.getBoegId())
        {
            // User is currently controlling the boeg, use boeg's location instead
            vertexId = gameState.getBoegPosition();
        }
        const auto &vertex = gameState.getVertices()[vertexId];
        const std::wstring userPositionMsg(vertex.location.begin(), vertex.location.end());
        text.drawAt(userPositionMsg, -0.5f*width, -0.5f*height + 20.0f, glm::vec3(0.0f), aspect);
        
        //for (const auto &vertex : gameState.getVertices())
        //{
        //    const std::string &loc = vertex.location;
        //    const std::wstring wloc (loc.begin(), loc.end());
        //    text.drawAtCentered(wloc, vertex.xpos*width*0.5f, vertex.ypos*height*0.5f, glm::vec3(0.0f, 0.0f, 1.0f), Text::CENTER_BOTH, aspect, 0.4);
        //}
        
        // Draw active targets of user player
        for (const auto target : userPlayer.getActiveTargets())
        {
            const auto &vertex = gameState.getVertices()[target];
            text.drawAtCentered(L"*", vertex.xpos*width*0.5f, vertex.ypos*height*0.5f, playerColors[userPlayer.getId()], Text::CENTER_BOTH, aspect);
        }
        
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
    if (graphics)
        graphics->framebufferResized = true;
}
    
void Graphics::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    (void)mods;  // unused
    
    auto graphics = reinterpret_cast<Graphics *>(glfwGetWindowUserPointer(window));
    if (graphics)
    {
        const bool isUsersTurn = graphics->gameState.checkIfUserTurn();
        
        if (isUsersTurn)
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            {
                // Left mouse button was pressed
                // Compute device coordinates from cursor pos
                GLdouble xpos = 0.0, ypos = 0.0;
                glfwGetCursorPos(window, &xpos, &ypos);
                
                GLint width = 0, height = 0;
                glfwGetFramebufferSize(window, &width, &height);        
                // Map from screen coordinates to device coordinates
                // TODO: Replace this with inline function
                xpos = 2.0 * xpos / width - 1.0;
                ypos = 1.0 - 2.0 * ypos / height;  // flip y-axis
                // Inverse orthographic projection (multiply by aspect ratio)
                xpos *= static_cast<GLdouble>(width) / height;
                        
                const GLdouble radius = graphics->circles.radius;
                
                // Check all vertices for potential collision with mouse click position
                const auto &vertices = graphics->gameState.getVertices();
                for (uint32_t i = 0; i < vertices.size(); ++i)
                {
                    const GLdouble dx = xpos - vertices[i].xpos;
                    const GLdouble dy = ypos - vertices[i].ypos;
                
                    if (dx*dx + dy*dy <= radius*radius)
                    {
                        // Set clicked vertex (circle) index needed for user strategy
                        graphics->gameState.setUserClickedPosition(i);
                        // Don't re-roll the dice if the user made an invalid move
                        const auto status = graphics->gameState.makeMove();
                        if (status != Game::TRY_AGAIN && status != Game::GAME_OVER)
                        {
                            graphics->gameState.prepareNextMove();
                        }
                        if (status != Game::TRY_AGAIN)
                        {
                            graphics->sound.play(Sound::SFX_MOVE);
                        }
                        else
                        {
                            graphics->sound.play(Sound::SFX_INVALID_MOVE);
                        }
                        // Start playing 'boeg theme' when user captures the boeg
                        if (status == Game::CAPTURE)
                        {
                            graphics->sound.play(Sound::SFX_CAPTURE);
                            graphics->sound.play(Sound::BOEG_THEME);
                        }
                        
                        break;  // circles must NOT overlap; stopping
                    }
                }
            }
        }
        else
        {
            if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
            {
                const auto boegId = graphics->gameState.getBoegId();
                const auto userId = graphics->gameState.getUserPlayer().getId();
                const bool wasUserBoeg = boegId == userId;
                // Advance state of game
                const auto status = graphics->gameState.makeMove();
                if (status != Game::GAME_OVER)
                {
                    graphics->gameState.prepareNextMove();
                }
                // If user was captured, play main theme again
                if (status == Game::CAPTURE)
                {
                    graphics->sound.play(Sound::SFX_CAPTURE);
                    if (wasUserBoeg)
                        graphics->sound.play(Sound::MAIN_THEME);
                }
                else
                {
                    graphics->sound.play(Sound::SFX_MOVE);
                }
            }
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
