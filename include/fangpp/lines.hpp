#ifndef LINES_HPP
#define LINES_HPP

#include "gl_common.hpp"

class Lines
{
public:
    struct Vertex
    {
        glm::vec2 pos;
        glm::vec3 col;
    };
    
    Lines();
    
    void updateProjection(const GLfloat width, const GLfloat height) const;
    
    void draw() const;
    
    ~Lines();
    
    static const constexpr GLuint nLines = 2;
    static const constexpr GLfloat lineWidth = 6.0f;
    static const constexpr GLfloat arrowLen = 0.1f;
    
private:
    GLuint vaoLines = 0;
    GLuint vaoArrow = 0;
    GLuint vboLines = 0;
    GLuint vboArrow = 0;
    GLuint shaderProgram = 0;
};

#endif /* LINES_HPP */
