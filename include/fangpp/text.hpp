#ifndef TEXT_HPP
#define TEXT_HPP

#include "gl_common.hpp"

#include <freetype2/ft2build.h>
#include <freetype2/freetype/fttypes.h>  // FT_UInt
#include FT_FREETYPE_H

#include <string>
#include <vector>
#include <array>

struct FontChar
{
    GLuint textureId;
    GLuint width;
    GLuint height;
    GLint bearingX;
    GLint bearingY;
    GLint advance;
};

class Text
{
public:

    enum CenteringType
    {
        CENTER_HORIZONTAL = 0,
        CENTER_VERTICAL,
        CENTER_BOTH
    };
    
    struct Vertex
    {
        glm::vec2 pos;
        glm::vec2 texCoord;        
    };
    
    struct Dimensions
    {
        GLfloat width;
        GLfloat height;
        GLfloat xmin;
        GLfloat ymin;
    };
    
    struct TextureData
    {
        Dimensions dims;
        GLuint textureId;
        GLfloat xspacing;
    };
    
    Text(const char *fontPath);
    
    void updateProjection(const GLfloat width, const GLfloat height) const;
    
    void drawAt(const std::wstring &text, GLfloat x, GLfloat y, 
        const glm::vec3 &color, GLfloat aspect = 1.0f, GLfloat scale = 1.0f);
    
    void drawAtCentered(const std::wstring &text, GLfloat cx, GLfloat cy, 
        const glm::vec3 &color, CenteringType type, 
        GLfloat aspect = 1.0f, GLfloat scale = 1.0f);
    
    Dimensions getDimensions(const std::wstring &text, GLfloat scale = 1.0f) const;
    
    ~Text();
    
    static const int numChars = 256;
    
private:
    void fontToTexture(const char *fontPath);
    
    TextureData getCharTextureData(const wchar_t c, GLfloat scale) const;
    
    GLuint vaoFont       = 0;
    GLuint vboFontVertex = 0;
    GLuint vboFontIndex  = 0;
    GLuint shaderProgram = 0;
    std::vector<FontChar> fontChars;
    std::array<Vertex, 4> vertices;  // quad to draw texture to
    std::array<GLubyte, 6> indices;  // index buffer for quad
};

#endif /* TEXT_HPP */
