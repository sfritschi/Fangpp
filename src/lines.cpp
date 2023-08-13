#include <fangpp/lines.hpp>
#include <fangpp/circles.hpp>  // radius

#include <array>
#include <stdexcept>
#include <iostream>

Lines::Lines()
{
    // Initialize shader program
    shaderProgram = createGLShaderProgram("shaders/lines/shader.vert",
                                          "shaders/lines/shader.frag");
    
    std::array<Vertex, 2 * nLines> vertices;
    vertices[0] = {{-0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}};
    vertices[1] = {{0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}};
    vertices[2] = {{0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}};
    vertices[3] = {{-0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}};
    
    // TODO: Move to separate function
    const GLfloat radius = Circles::radius;
    // Vertices of base triangle
    const GLfloat height = std::tan(glm::radians(30.0f)) * arrowLen;
    const glm::mat3x2 triangle (0.0f, 0.0f, -arrowLen, height, -arrowLen, -height);
    // Fill vertices of 'arrow' triangles
    std::array<Vertex, 3 * nLines> verticesTriangle;
    for (GLuint i = 0; i < nLines; ++i)
    {
        const glm::vec3 col = vertices[2 * i    ].col;
        const glm::vec2 p1  = vertices[2 * i    ].pos;
        const glm::vec2 p2  = vertices[2 * i + 1].pos;
        
        const glm::vec2 diff = p2 - p1;
        const GLfloat l = glm::length(diff);
        const GLfloat t = radius / l;
        const GLfloat q = (radius + arrowLen * 2.0f / 3.0f) / l;
        // Translation vector for arrow (triangle)
        const glm::vec2 trans = t * p1 + (1.0f - t) * p2;
        
        // Update edge end position to end at center of arrow (triangle)
        vertices[2 * i + 1].pos = q * p1 + (1.0f - q) * p2;
        
        // Find cos and sin of angle that edge makes between positive x-axis
        const GLfloat c = diff.x / l;
        const GLfloat s = diff.y / l;
        
        const glm::mat2 R (c, s, c, -s);  // rotation matrix
        
        for (GLuint j = 0; j < 3; ++j)
        {
            const GLuint index = 3 * i + j;
            const glm::vec2 rot = R * glm::column(triangle, j) + trans;
            verticesTriangle[index].pos = rot;
            verticesTriangle[index].col = col;
        }
    }
    
    CHKERRGL(glUseProgram(shaderProgram));
    {
        CHKERRGL(glGenVertexArrays(1, &vaoLines));
        CHKERRGL(glBindVertexArray(vaoLines));
        {
            CHKERRGL(glGenBuffers(1, &vboLines));
            CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, vboLines));
            {
                CHKERRGL(glBufferData(GL_ARRAY_BUFFER,
                                      vertices.size() * sizeof(vertices[0]),
                                      vertices.data(),
                                      GL_STATIC_DRAW
                ));
                // See 'aPos' attribute in vshader
                CHKERRGL(glVertexAttribPointer(0,
                                               2,
                                               GL_FLOAT,
                                               GL_FALSE,
                                               sizeof(vertices[0]),
                                               (const void *)offsetof(Vertex, pos)
                ));
                CHKERRGL(glEnableVertexAttribArray(0));
                // See 'aCol' attribute in vshader
                CHKERRGL(glVertexAttribPointer(1,
                                               3,
                                               GL_FLOAT,
                                               GL_FALSE,
                                               sizeof(vertices[0]),
                                               (const void *)offsetof(Vertex, col)
                ));
                CHKERRGL(glEnableVertexAttribArray(1));
            }
            CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        }
        CHKERRGL(glBindVertexArray(0));
        
        CHKERRGL(glGenVertexArrays(1, &vaoArrow));
        CHKERRGL(glBindVertexArray(vaoArrow));
        {
            CHKERRGL(glGenBuffers(1, &vboArrow));
            CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, vboArrow));
            {
                CHKERRGL(glBufferData(GL_ARRAY_BUFFER,
                                      verticesTriangle.size() * sizeof(verticesTriangle[0]),
                                      verticesTriangle.data(),
                                      GL_STATIC_DRAW
                ));
                // See 'aPos' attribute in vshader
                CHKERRGL(glVertexAttribPointer(0,
                                               2,
                                               GL_FLOAT,
                                               GL_FALSE,
                                               sizeof(verticesTriangle[0]),
                                               (const void *)offsetof(Vertex, pos)
                ));
                CHKERRGL(glEnableVertexAttribArray(0));
                // See 'aCol' attribute in vshader
                CHKERRGL(glVertexAttribPointer(1,
                                               3,
                                               GL_FLOAT,
                                               GL_FALSE,
                                               sizeof(verticesTriangle[0]),
                                               (const void *)offsetof(Vertex, col)
                ));
                CHKERRGL(glEnableVertexAttribArray(1));
            }
            CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        }
        CHKERRGL(glBindVertexArray(0));
    }
    CHKERRGL(glUseProgram(0));
}

void Lines::updateProjection(const GLfloat width, const GLfloat height) const
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

void Lines::draw() const
{
    CHKERRGL(glUseProgram(shaderProgram));
    {
        // Draw edges
        CHKERRGL(glBindVertexArray(vaoLines));
        {
            CHKERRGL(glLineWidth(lineWidth));
            CHKERRGL(glDrawArrays(GL_LINES, 0, 2 * nLines));
        }
        CHKERRGL(glBindVertexArray(0));
        // Draw arrows
        CHKERRGL(glBindVertexArray(vaoArrow));
        {
            CHKERRGL(glDrawArrays(GL_TRIANGLES, 0, 3 * nLines));
        }
        CHKERRGL(glBindVertexArray(0));
    }
    CHKERRGL(glUseProgram(0));
}

Lines::~Lines()
{
    try
    {
        CHKERRGL(glUseProgram(0));
        CHKERRGL(glDisableVertexAttribArray(0));
        CHKERRGL(glDisableVertexAttribArray(1));
        CHKERRGL(glDeleteBuffers(1, &vboLines));
        CHKERRGL(glDeleteBuffers(1, &vboArrow));
        CHKERRGL(glDeleteVertexArrays(1, &vaoLines));
        CHKERRGL(glDeleteVertexArrays(1, &vaoArrow));
        CHKERRGL(glDeleteProgram(shaderProgram));
    }
    catch (const std::exception &e)
    {
        std::cerr << "GL Error: " << e.what() << '\n';
    }
    
}
