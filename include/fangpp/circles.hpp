#ifndef CIRCLES_HPP
#define CIRCLES_HPP

#include "gl_common.hpp"
#include "graph.hpp"


class Circles
{
public:
    Circles(const std::vector<Vertex> &vertices);
    
    void updateProjection(const GLfloat width, const GLfloat height) const;
    
    void draw() const;
    
    void animateColors(const GLfloat currentTime, const std::array<uint32_t, 7> &positions) const;
    
    ~Circles();
    
    static constexpr const GLfloat radius          = 0.03f;
    static constexpr const GLuint nTrianglesCircle = 64;
    
private:
    GLuint nInstances              = 0;
    GLuint vaoCircles              = 0;
    GLuint vboCirclesVert          = 0;
    GLuint vboCirclesInstancePos   = 0;
    GLuint vboCirclesInstanceColor = 0;
    GLuint shaderProgram           = 0;
};

#endif /* CIRCLES_HPP */
