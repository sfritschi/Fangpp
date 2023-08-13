#ifndef CIRCLES_HPP
#define CIRCLES_HPP

#include "gl_common.hpp"

class Circles
{
public:
    Circles();
    
    void updateProjection(const GLfloat width, const GLfloat height) const;
    
    void draw() const;
    
    void updateColors() const;
    
    ~Circles();
    
    static constexpr const GLfloat radius          = 0.25f;
    static constexpr const GLuint nTrianglesCircle = 64;
    static constexpr const GLuint nInstances       = 4;
    
private:
    GLuint vaoCircles              = 0;
    GLuint vboCirclesVert          = 0;
    GLuint vboCirclesInstancePos   = 0;
    GLuint vboCirclesInstanceColor = 0;
    GLuint shaderProgram           = 0;
};

#endif /* CIRCLES_HPP */
