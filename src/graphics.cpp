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
    glfwSetCursorPosCallback(window, cursorPosCallback);
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
        // TODO: Move text rendering to separate function
        // Draw HUD text describing the current state of the game
        const auto &userPlayer = gameState.getUserPlayer();
        const auto playerId = gameState.getCurrentPlayer().getId();
        
        glm::vec3 playerColor = playerColors[playerId];
        if (playerId == gameState.getBoegId())
        {
            playerColor = playerColors[nPlayableCharacters - 1];  // color of boeg
        }
        
        if (userPlayer.getId() == playerId)
        {
            const std::wstring diceRollMsg1 = L"You rolled a " + std::to_wstring(gameState.getDiceRoll());
            text.drawAt(diceRollMsg1, -0.5f*width, 0.5f*height - 40.0f, playerColor, aspect);
        }
        else
        {
            const std::wstring diceRollMsg1 = L"Player " + std::to_wstring(playerId + 1) + L" rolled a " + std::to_wstring(gameState.getDiceRoll());
            text.drawAt(diceRollMsg1, -0.5f*width, 0.5f*height - 40.0f, playerColor, aspect);
        }
        
        // Draw name of the location where the user is currently
        uint32_t vertexId = userPlayer.getPosition();
        glm::vec3 userColor = playerColors[userPlayer.getId()];
        if (userPlayer.getId() == gameState.getBoegId())
        {
            // User is currently controlling the boeg, use boeg's location instead
            vertexId = gameState.getBoegPosition();
            userColor = playerColors[nPlayableCharacters - 1];
        }
        const auto &vertex = gameState.getVertices()[vertexId];
        const std::wstring userPositionMsg(vertex.location.begin(), vertex.location.end());
        text.drawAt(userPositionMsg, -0.5f*width, -0.5f*height + 20.0f, userColor, aspect);
        
        // Draw the number of active targets for each player
        GLfloat offset = 150.0f; GLfloat spacing = 5.0f;
        const GLfloat targetTextScale = 0.75f;
        const std::wstring msg = L"Targets left:";
        const auto d = text.getDimensions(msg);
        text.drawAt(msg, -0.5f*width, -offset, glm::vec3(0.0f), aspect, targetTextScale);
        
        offset += d.height + spacing;
        for (const Player &player : gameState.getPlayers())
        {
            const uint32_t nActiveTargets = player.getActiveTargets().size();
            const std::wstring targetMsg = L"Player " + std::to_wstring(player.getId() + 1) + L": " + std::to_wstring(nActiveTargets);
            const auto dims = text.getDimensions(targetMsg);
            
            glm::vec3 color = playerColors[player.getId()];
            if (player.isBoeg(gameState))
            {
                color = playerColors[nPlayableCharacters - 1];  // color of boeg
            }
            text.drawAt(targetMsg, -0.5f*width, -offset, color, aspect, targetTextScale);
            
            offset += dims.height + spacing;
        }
        
        // Draw active targets of user
        for (const auto target : userPlayer.getActiveTargets())
        {
            const auto &vertex = gameState.getVertices()[target];
            text.drawAtCentered(L"*", vertex.xpos*width*0.5f, vertex.ypos*height*0.5f, playerColors[userPlayer.getId()], Text::CENTER_BOTH, aspect);
        }
        
        if (hoverLocationIndex)
        {
            // TODO: It is still somewhat hard to read the location name.
            //       Either find a better contrasting color, or implement an
            //       outline/border color for text
            const glm::vec3 color(64.0f / 255.0f, 54.0f / 255.0f, 43.0f / 255.0f);
            const uint32_t location = hoverLocationIndex.value();
            const auto &vertex = gameState.getVertices()[location];
            const std::wstring locationMsg (vertex.location.begin(), vertex.location.end());
            text.drawAtCentered(locationMsg, vertex.xpos*width*0.5f, vertex.ypos*height*0.5f, color, Text::CENTER_BOTH, aspect, 0.5f);
        }
        
        if (gameState.isGameOver())
        {
            const glm::vec3 color(64.0f / 255.0f, 54.0f / 255.0f, 43.0f / 255.0f);
            text.drawAtCentered(L"Game Over! Press 'R' to restart", 0.0f, 0.0f, color, Text::CENTER_BOTH, aspect, 1.5f);
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Graphics::playStatusSound(Game::Status status, bool wasUserPlayingAsBoeg)
{
    const bool isUser = gameState.checkIfUserTurn();
    if (isUser && (status & Game::TRY_AGAIN))
    {
        sound.play(Sound::SFX_INVALID_MOVE);
    }
    else if ((status & Game::CONTINUE) || (status & Game::GAME_OVER))
    {
        if (status & Game::CAPTURE)
        {
            if (isUser)
            {
                sound.play(Sound::BOEG_THEME);
            }
            else if (wasUserPlayingAsBoeg)
            {
                // If user was captured, play main theme again
                sound.play(Sound::MAIN_THEME);
            }
            sound.play(Sound::SFX_CAPTURE);
            if (status & Game::TARGET_VISITED)
            {
                // TODO: Maybe wait for completion of capture sound first
                sound.play(Sound::SFX_TARGET);
            }
        }
        else if (status & Game::TARGET_VISITED)
        {
            sound.play(Sound::SFX_TARGET);
        }
        else
        {
            sound.play(Sound::SFX_MOVE);
        }
    }    
}

uint32_t Graphics::getClickedVertexByIndex() const
{
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
            
    const GLdouble radius = circles.radius;
    
    // Check all vertices for potential collision with mouse click position
    const auto &vertices = gameState.getVertices();
    for (uint32_t i = 0; i < vertices.size(); ++i)
    {
        const GLdouble dx = xpos - vertices[i].xpos;
        const GLdouble dy = ypos - vertices[i].ypos;
    
        if (dx*dx + dy*dy <= radius*radius)
        {
            return i;
        }
    }
    
    return std::numeric_limits<uint32_t>::max();  // no vertex was clicked
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
        if (graphics->gameState.isGameOver())
        {
            return;  // nothing to do...
        }
        
        const bool isUsersTurn = graphics->gameState.checkIfUserTurn();
        
        if (isUsersTurn)
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            {
                const uint32_t vertexIndex = graphics->getClickedVertexByIndex();
                if (vertexIndex < graphics->gameState.getNVertices())
                {
                    // Set clicked vertex (circle) index needed for user strategy
                    graphics->gameState.setUserClickedPosition(vertexIndex);
                    // Advance state of game
                    const auto status = graphics->gameState.makeMove();
                    graphics->playStatusSound(status, false);
                    // Don't re-roll the dice if the user made an invalid move
                    if (!(status & Game::TRY_AGAIN) && !(status & Game::GAME_OVER))
                    {
                        graphics->gameState.prepareNextMove(status);
                    }
                }
            }
        }
        else
        {
            if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
            {
                const bool wasUserPlayingAsBoeg = graphics->gameState.isUserPlayingAsBoeg();
                // Advance state of game
                const auto status = graphics->gameState.makeMove();
                graphics->playStatusSound(status, wasUserPlayingAsBoeg);
                
                if (!(status & Game::GAME_OVER))
                {
                    graphics->gameState.prepareNextMove(status);
                }
            }
        }
    }
}

void Graphics::cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    (void) xpos;  // unused
    (void) ypos;  // unused
    
    auto graphics = reinterpret_cast<Graphics *>(glfwGetWindowUserPointer(window));
    if (graphics)
    {
        const uint32_t vertexIndex = graphics->getClickedVertexByIndex();
        if (vertexIndex < graphics->gameState.getNVertices())
        {
            graphics->hoverLocationIndex = vertexIndex;
        }
        else
        {
            graphics->hoverLocationIndex.reset();
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
    
    auto graphics = reinterpret_cast<Graphics *>(glfwGetWindowUserPointer(window));
    if (graphics && graphics->gameState.isGameOver())
    {
        if (key == GLFW_KEY_R && action == GLFW_PRESS)
        {
            // Begin a new game by re-initializing the state
            graphics->gameState.initializeState();
            // Reset sound engine state to beginning 
            graphics->sound.reset();
        }
    }
}

void Graphics::glfwError(GLint error, const GLchar *description)
{
    (void)error;  // unused
    
    throw std::runtime_error("GLFW error: " + std::string(description));
}
