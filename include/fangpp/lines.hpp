#ifndef LINES_HPP
#define LINES_HPP

#include "gl_common.hpp"
#include "graph.hpp"

class Lines
{
public:
    Lines(const std::vector<LineVertex> &lines);
    
    void updateProjection(const GLfloat width, const GLfloat height) const;
    
    void draw() const;
    
    ~Lines();
    
    static const constexpr GLfloat lineWidth = 3.0f;
    static const constexpr GLfloat arrowLen = 0.1f;
    
private:
    GLuint nLines = 0;
    GLuint vaoLines = 0;
    //GLuint vaoArrow = 0;
    GLuint vboLines = 0;
    //GLuint vboArrow = 0;
    GLuint shaderProgram = 0;
};

#endif /* LINES_HPP */
