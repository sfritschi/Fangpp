#include <iostream>
#include <array>
#include <stdexcept>

#include <fangpp/circles.hpp>

Circles::Circles(const std::vector<Vertex> &vertices) : nInstances(vertices.size())
{
    shaderProgram = createGLShaderProgram("shaders/circles/shader.vert",
                                          "shaders/circles/shader.frag");
    
    const GLfloat angleIncrement = 2.0f * glm::pi<GLfloat>() / nTrianglesCircle;
    
    std::array<glm::vec2, nTrianglesCircle + 2> circleVertices;
    // Set center of circle
    circleVertices[0] = glm::vec2(0.0f);
    for (GLuint i = 0; i <= nTrianglesCircle; ++i)
    {
        // Compute position on circle perimeter
        const GLfloat angle = i * angleIncrement;
        circleVertices[i + 1] = glm::vec2(
            radius * std::cos(angle), radius * std::sin(angle)
        );
    }
    
    // Define static circle colors: Black for station vertices and
    // magenta for target vertices
    std::vector<glm::vec3> instanceColors(vertices.size());
    for (uint32_t i = 0; i < vertices.size(); ++i)
    {
        if (vertices[i].isTarget)
        {
            instanceColors[i] = glm::vec3(1.f, 92.f/255.f, 244.f/255.f);  // pink
        }
        else
        {
            instanceColors[i] = glm::vec3(0.0f);  // black
        }
    }
    
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
                                      vertices.size() * sizeof(vertices[0]),
                                      vertices.data(),
                                      GL_STATIC_DRAW
                ));
                CHKERRGL(glVertexAttribPointer(1, // location in shader
                                               2, // #elements of attribute
                                               GL_FLOAT, // type of attribute
                                               GL_FALSE, // normalize?
                                               sizeof(Vertex),  // stride
                                               (const void *)offsetof(Vertex, xpos))  // offset
                ); 
                CHKERRGL(glEnableVertexAttribArray(1));
                CHKERRGL(glVertexAttribDivisor(1, 1));  // location, instance increment
            }
            CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            
            CHKERRGL(glGenBuffers(1, &vboCirclesInstanceColor));
            CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, vboCirclesInstanceColor));
            {
                CHKERRGL(glBufferData(GL_ARRAY_BUFFER,
                                      instanceColors.size() * sizeof(instanceColors[0]),
                                      instanceColors.data(),
                                      GL_STATIC_DRAW
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

void Circles::animateColors(const GLfloat currentTime, const std::array<uint32_t, 7> &positions) const
{
    CHKERRGL(glUseProgram(shaderProgram));
    {
        GLint playerPositionsLocation;
        CHKERRGL(playerPositionsLocation = glGetUniformLocation(shaderProgram, "playerPositions"));
        CHKERRGL(glUniform1uiv(playerPositionsLocation, positions.size(), positions.data()));
        
        GLint currentTimeLocation;
        CHKERRGL(currentTimeLocation = glGetUniformLocation(shaderProgram, "currentTime"));
        CHKERRGL(glUniform1f(currentTimeLocation, currentTime));
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
