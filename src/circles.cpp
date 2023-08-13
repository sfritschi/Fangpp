#include <iostream>
#include <array>
#include <stdexcept>

#include <fangpp/circles.hpp>

Circles::Circles()
{
    shaderProgram = createGLShaderProgram("shaders/circles/shader.vert",
                                          "shaders/circles/shader.frag");
    
    const GLfloat angleIncrement = 2.0f * glm::pi<GLfloat>() / nTrianglesCircle;
    
    std::array<glm::vec2, nTrianglesCircle + 2> circleVertices;
    // Set center of circle
    circleVertices[0] = glm::vec2(0.0f, 0.0f);
    for (GLuint i = 0; i <= nTrianglesCircle; ++i)
    {
        // Compute position on circle perimeter
        const GLfloat angle = i * angleIncrement;
        circleVertices[i + 1] = glm::vec2(
            radius * std::cos(angle), radius * std::sin(angle)
        );
    }
    
    std::array<glm::vec2, nInstances> instancePositions;
    instancePositions[0] = {-0.5f, -0.5f};
    instancePositions[1] = {0.5f, -0.5f};
    instancePositions[2] = {0.5f, 0.5f};
    instancePositions[3] = {-0.5f, 0.5f};
    
    std::array<glm::vec3, nInstances> instanceColors;
    instanceColors[0] = {1.0f, 0.0f, 0.0f};
    instanceColors[1] = {0.0f, 1.0f, 0.0f};
    instanceColors[2] = {0.0f, 0.0f, 1.0f};
    instanceColors[3] = {1.0f, 1.0f, 0.0f};
    
    CHKERRGL(glUseProgram(shaderProgram));
    {
        CHKERRGL(glGenVertexArrays(1, &vaoCircles));
        CHKERRGL(glBindVertexArray(vaoCircles));
        {
            CHKERRGL(glGenBuffers(1, &vboCirclesVert));
            CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, vboCirclesVert));
            {
                // Set buffer data
                CHKERRGL(glBufferData(GL_ARRAY_BUFFER, 
                                      circleVertices.size() * sizeof(circleVertices[0]),
                                      circleVertices.data(),
                                      GL_STATIC_DRAW
                ));
                // Enable respective vertex attributes
                CHKERRGL(glVertexAttribPointer(0, // location in shader
                                               2, // #elements of attribute
                                               GL_FLOAT, // type of attribute
                                               GL_FALSE, // normalize?
                                               sizeof(circleVertices[0]),  // stride
                                               nullptr)  // offset
                ); 
                CHKERRGL(glEnableVertexAttribArray(0));
            }
            CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            
            CHKERRGL(glGenBuffers(1, &vboCirclesInstancePos));
            CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, vboCirclesInstancePos));
            {
                CHKERRGL(glBufferData(GL_ARRAY_BUFFER,
                                      instancePositions.size() * sizeof(instancePositions[0]),
                                      instancePositions.data(),
                                      GL_STATIC_DRAW
                ));
                CHKERRGL(glVertexAttribPointer(1, // location in shader
                                               2, // #elements of attribute
                                               GL_FLOAT, // type of attribute
                                               GL_FALSE, // normalize?
                                               sizeof(instancePositions[0]),  // stride
                                               nullptr)  // offset
                ); 
                CHKERRGL(glEnableVertexAttribArray(1));
                CHKERRGL(glVertexAttribDivisor(1, 1));  // location, instance increment
            }
            CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            
            CHKERRGL(glGenBuffers(1, &vboCirclesInstanceColor));
            CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, vboCirclesInstanceColor));
            {
                // Note: Instance color buffer is used for dynamic drawing
                CHKERRGL(glBufferData(GL_ARRAY_BUFFER,
                                      instanceColors.size() * sizeof(instanceColors[0]),
                                      instanceColors.data(),
                                      GL_DYNAMIC_DRAW
                ));
                CHKERRGL(glVertexAttribPointer(2, // location in shader
                                               3, // #elements of attribute
                                               GL_FLOAT, // type of attribute
                                               GL_FALSE, // normalize?
                                               sizeof(instanceColors[0]),  // stride
                                               nullptr)  // offset
                ); 
                CHKERRGL(glEnableVertexAttribArray(2));
                CHKERRGL(glVertexAttribDivisor(2, 1));
            }
            CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        }
        CHKERRGL(glBindVertexArray(0));
    }
    CHKERRGL(glUseProgram(0));
}

void Circles::draw() const
{
    // Use this shader program
    CHKERRGL(glUseProgram(shaderProgram));
    {
        CHKERRGL(glBindVertexArray(vaoCircles));
        {
            // Note: GL_TRIANGLE_FAN draws N - 2 triangles
            // TODO: Replace hard coded number of instances == 4
            CHKERRGL(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, nTrianglesCircle + 2, nInstances)); 
        }
        CHKERRGL(glBindVertexArray(0));
    }
    CHKERRGL(glUseProgram(0));
}

void Circles::updateProjection(const GLfloat width, const GLfloat height) const
{
    CHKERRGL(glUseProgram(shaderProgram));
    {
        const GLfloat aspect = width / height;
        const glm::mat4 proj = glm::ortho(-aspect, aspect, -1.0f, 1.0f);
        
        GLint projLocation;
        CHKERRGL(projLocation = glGetUniformLocation(shaderProgram, "proj"));
        CHKERRGL(glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(proj)));
    }
    CHKERRGL(glUseProgram(0));
}

void Circles::updateColors() const
{
    CHKERRGL(glUseProgram(shaderProgram));
    {
        CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, vboCirclesInstanceColor));
        {
            std::array<glm::vec3, nInstances> colors;
            // Set all circles to green color
            colors[0] = glm::vec3(0.0f, 1.0f, 0.0f);
            colors[1] = glm::vec3(0.0f, 1.0f, 0.0f);
            colors[2] = glm::vec3(0.0f, 1.0f, 0.0f);
            colors[3] = glm::vec3(0.0f, 1.0f, 0.0f);
            // Update contents of per instance color array buffer
            CHKERRGL(glBufferSubData(GL_ARRAY_BUFFER,
                                     0,
                                     colors.size() * sizeof(colors[0]),
                                     colors.data()));
        }
        CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }
    CHKERRGL(glUseProgram(0));
}

Circles::~Circles()
{
    try
    {
        CHKERRGL(glUseProgram(0));
        CHKERRGL(glDisableVertexAttribArray(0));
        CHKERRGL(glDisableVertexAttribArray(1));
        CHKERRGL(glDisableVertexAttribArray(2));
        CHKERRGL(glDeleteBuffers(1, &vboCirclesVert));
        CHKERRGL(glDeleteBuffers(1, &vboCirclesInstancePos));
        CHKERRGL(glDeleteBuffers(1, &vboCirclesInstanceColor));
        CHKERRGL(glDeleteVertexArrays(1, &vaoCircles));
        CHKERRGL(glDeleteProgram(shaderProgram));
    } 
    catch(const std::exception &e)
    {
        std::cerr << "GL Error: " << e.what() << '\n';
    }
}
