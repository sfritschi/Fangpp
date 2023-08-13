#include <fangpp/text.hpp>

#include <stdexcept>
#include <iostream>
#include <limits>  // std::numeric_limits::infinity
#include <algorithm>  // std::max, std::min

Text::Text(const char *fontPath)
{
    // Initialize shader program
    shaderProgram = createGLShaderProgram("shaders/text/shader.vert",
                                          "shaders/text/shader.frag");
    CHKERRGL(glUseProgram(shaderProgram));
    {
        // Create textures for given font
        fontToTexture(fontPath);
        
        CHKERRGL(glGenVertexArrays(1, &vaoFont));
        CHKERRGL(glBindVertexArray(vaoFont));
        {
            CHKERRGL(glGenBuffers(1, &vboFontVertex));
            CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, vboFontVertex));
            {
                // Initialize vertex buffer data
                CHKERRGL(glBufferData(GL_ARRAY_BUFFER,
                                      vertices.size() * sizeof(vertices[0]),
                                      nullptr,  // no data available yet
                                      GL_DYNAMIC_DRAW
                ));
                // See 'aPos' in vertex shader
                CHKERRGL(glVertexAttribPointer(0,
                                               2,
                                               GL_FLOAT,
                                               GL_FALSE,
                                               sizeof(vertices[0]),
                                               (const void *)offsetof(Vertex, pos)
                ));
                CHKERRGL(glEnableVertexAttribArray(0));
                // See 'aTex in vertex shader
                CHKERRGL(glVertexAttribPointer(1,
                                               2,
                                               GL_FLOAT,
                                               GL_FALSE,
                                               sizeof(vertices[0]),
                                               (const void *)offsetof(Vertex, texCoord)
                ));
                CHKERRGL(glEnableVertexAttribArray(1));
            }
            CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            
            CHKERRGL(glGenBuffers(1, &vboFontIndex));
            CHKERRGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboFontIndex));
            {
                // Initialize index buffer data
                CHKERRGL(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                                      indices.size() * sizeof(indices[0]),
                                      nullptr,  // no data available yet
                                      GL_DYNAMIC_DRAW
                ));
                
            }
            CHKERRGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        }
        CHKERRGL(glBindVertexArray(0));
    }
    CHKERRGL(glUseProgram(0));
}

void Text::fontToTexture(const char *fontPath)
{
    // Initialize FT library
    FT_Library ft;
    FT_Face face;
    FT_Error err;
    
    err = FT_Init_FreeType(&ft);
    if (err != FT_Err_Ok)
    {
        throw std::runtime_error("Failed to initialize FreeType\n");
    }
    
    // Load font face from file
    err = FT_New_Face(ft, fontPath, 0, &face);  // note: use index 0 (multiple faces)
    if (err != FT_Err_Ok)
    {
        throw std::runtime_error("Failed to load font face from: " + 
            std::string(fontPath));
    }
    err = FT_Set_Pixel_Sizes(face,
                             0,    // width; 0 means same as height below
                             32);  // height; 32 * 16 pixels
    if (err != FT_Err_Ok)
    {
        throw std::runtime_error("Failed to set pixel size");
    }
    // Unpacking of pixel data from (client) memory using BYTE-alignment
    // for start of each pixel row
    CHKERRGL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    
    fontChars.resize(numChars);
    for (wchar_t c = 0; c < numChars; ++c)
    {
        const FT_UInt index = static_cast<FT_UInt>(c);
        
        err = FT_Load_Char(face, c, FT_LOAD_RENDER);
        if (err != FT_Err_Ok)
        {
            fontChars[index] = {};
            std::wcerr << "Warning: Failed to load char: " << c << '\n';
            continue;  // move on
        }
        
        // Create 2D texture from glyph slot
        GLuint texture;
        CHKERRGL(glGenTextures(1, &texture));
        CHKERRGL(glBindTexture(GL_TEXTURE_2D, texture));
        {
            CHKERRGL(glTexImage2D(
                GL_TEXTURE_2D,  // type of texture
                0,              // level of detail (only base)
                GL_RED,         // single component, clamped to [0, 1]
                face->glyph->bitmap.width,  // texture width
                face->glyph->bitmap.rows,   // texture height
                0,              // border (must be 0)
                GL_RED,         // pixel data format
                GL_UNSIGNED_BYTE,  // pixel data type
                static_cast<const void *>(face->glyph->bitmap.buffer)  // image data
            ));
            // Set options for texture (no mip-mapping)
            CHKERRGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            CHKERRGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            // Clamp texture coord u to edge
            CHKERRGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            // Clamp texture coord v to edge
            CHKERRGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            
            // Store glyph texture data
            fontChars[index] = {
                .textureId = texture,
                .width = face->glyph->bitmap.width,
                .height = face->glyph->bitmap.rows,
                .bearingX = face->glyph->bitmap_left,
                .bearingY = face->glyph->bitmap_top,
                .advance = static_cast<GLint>(face->glyph->advance.x)
            };
        }
        CHKERRGL(glBindTexture(GL_TEXTURE_2D, 0));  // unbind again
    }
    
    // Cleanup
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

Text::TextureData Text::getCharTextureData(const wchar_t c, GLfloat scale) const
{
    const FT_UInt index = static_cast<FT_UInt>(c);
    if (index >= fontChars.size())
    {
        throw std::runtime_error("Invalid char index encountered");
    }
    
    const FontChar fc = fontChars[index];
    TextureData tdata = {
        .dims = {
            // Compute dimensions of quad
            .width  = fc.width * scale,
            .height = fc.height * scale,
            // Compute bottom-left corner of quad
            .xmin   = fc.bearingX * scale,
            // Note: fc.height > fc.bearingY
            .ymin   = -(static_cast<GLfloat>(fc.height) - fc.bearingY) * scale
        },
        .textureId  = fc.textureId,
        .xspacing   = (fc.advance >> 6) * scale
    };
    
    return tdata;
}

void Text::drawAt(const std::wstring &text, GLfloat x, GLfloat y, 
    const glm::vec3 &color, GLfloat aspect /* = 1.0f */, GLfloat scale /* = 1.0f */)
{
    CHKERRGL(glUseProgram(shaderProgram));
    {
        // Set text color in uniform location
        GLint textColorLocation;
        CHKERRGL(textColorLocation = glGetUniformLocation(shaderProgram, "textColor"));
        CHKERRGL(glUniform3fv(textColorLocation, 1, glm::value_ptr(color)));
        // Activate texture unit
        CHKERRGL(glActiveTexture(GL_TEXTURE0));
        // Prepare drawing command
        CHKERRGL(glBindVertexArray(vaoFont));
        {
            x /= aspect;  // orthographic projection
            
            for (const wchar_t c : text)
            {                
                const TextureData tdata = getCharTextureData(c, scale);
                
                const GLfloat xp = x + tdata.dims.xmin;
                const GLfloat yp = y + tdata.dims.ymin;
                const GLfloat w  = tdata.dims.width;
                const GLfloat h  = tdata.dims.height;
                
                // Note: Could make indices array static
                // Initialize vertex and index buffers
                vertices[0] = {{xp,     yp + h}, {0.0f, 0.0f}};  // 0: top-left
                vertices[1] = {{xp,     yp    }, {0.0f, 1.0f}};  // 1: bottom-left
                vertices[2] = {{xp + w, yp    }, {1.0f, 1.0f}};  // 2: bottom-right
                vertices[3] = {{xp + w, yp + h}, {1.0f, 0.0f}};  // 3: top-right
                indices = {1, 2, 3, 1, 3, 0};
                
                // Render glyph texture over quad
                CHKERRGL(glBindTexture(GL_TEXTURE_2D, tdata.textureId));
                
                CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, vboFontVertex));
                {
                    CHKERRGL(glBufferSubData(GL_ARRAY_BUFFER, 
                                             0, 
                                             vertices.size() * sizeof(vertices[0]),
                                             vertices.data()
                    ));
                }
                CHKERRGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
                
                CHKERRGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboFontIndex));
                {
                    CHKERRGL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 
                                             0, 
                                             indices.size() * sizeof(indices[0]),
                                             indices.data()
                    ));
                    
                    // Issue draw command while index buffer is still bound
                    CHKERRGL(glDrawElements(GL_TRIANGLES, 
                                            indices.size(), 
                                            GL_UNSIGNED_BYTE, 
                                            (void *)0
                    ));
                }
                CHKERRGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
                
                // Update cursor position
                x += tdata.xspacing;
            }
        }
        CHKERRGL(glBindVertexArray(0));
        CHKERRGL(glBindTexture(GL_TEXTURE_2D, 0));
    }
    CHKERRGL(glUseProgram(0));
}

void Text::drawAtCentered(const std::wstring &text, GLfloat cx, GLfloat cy, 
    const glm::vec3 &color, CenteringType type, 
    GLfloat aspect /* = 1.0f */, GLfloat scale /* = 1.0f */)
{
    // Compute dimensions of text
    Dimensions dims = getDimensions(text, scale);
    // Render text s.t. bounding box center coincides with (cx, cy)
    // NOTE: Need to multiply width with aspect ratio to preserve it
    dims.width *= aspect;
    
    GLfloat x, y;
    switch (type)
    {
        case CENTER_BOTH:
            x = cx - 0.5f * dims.width - dims.xmin;
            y = cy - 0.5f * dims.height - dims.ymin;
            break;
        
        case CENTER_HORIZONTAL:
            x = cx - 0.5f * dims.width - dims.xmin;
            y = cy;
            break;
        
        case CENTER_VERTICAL:
            x = cx;
            y = cy - 0.5f * dims.height - dims.ymin;
            break;
        
        default:
            throw std::runtime_error("Unrecognized centering type");
    }
    
    drawAt(text, x, y, color, aspect, scale);
}

Text::Dimensions Text::getDimensions(const std::wstring &text, 
    GLfloat scale /* = 1.0f */) const
{
    const GLfloat inf = std::numeric_limits<GLfloat>::infinity();
    GLfloat xmin = inf;
    GLfloat ymin = inf;
    GLfloat xmax = -inf;
    GLfloat ymax = -inf;
    
    GLfloat x = 0.0f;  // initialize
    for (const wchar_t c : text)
    {
        const TextureData tdata = getCharTextureData(c, scale);
        // Compute bottom-left position and dimensions of glyph quad
        const GLfloat xp = x + tdata.dims.xmin;
        const GLfloat yp = tdata.dims.ymin;
        const GLfloat w  = tdata.dims.width;
        const GLfloat h  = tdata.dims.height;
        
        // Note: xmin/xmax will trivially be xp of first/ (xp + w) of last char
        xmin = std::min(xmin, xp);
        ymin = std::min(ymin, yp);
        xmax = std::max(xmax, xp + w);
        ymax = std::max(ymax, yp + h);
        // Advance
        x += tdata.xspacing;
    }
    
    return { .width = xmax - xmin, 
             .height = ymax - ymin,
             .xmin = xmin,
             .ymin = ymin };
}

void Text::updateProjection(const GLfloat width, const GLfloat height) const
{
    // Compute orthographic projection matrix based on aspect ratio
    CHKERRGL(glUseProgram(shaderProgram));
    {
        // TODO: Projection matrix should be equivalent as for nodes
        //const GLfloat aspect = width / height;
        const glm::mat4 proj = glm::ortho(-0.5f*width, 0.5f*width, -0.5f*height, 0.5f*height);
        
        GLint projLocation;
        CHKERRGL(projLocation = glGetUniformLocation(shaderProgram, "proj"));
        CHKERRGL(glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(proj)));
    }
    CHKERRGL(glUseProgram(0));
}

Text::~Text()
{
    try
    {
        CHKERRGL(glUseProgram(0));
        CHKERRGL(glBindVertexArray(0));
        CHKERRGL(glDisableVertexAttribArray(0));
        CHKERRGL(glDisableVertexAttribArray(1));
        // Delete textures
        for (const auto &fc : fontChars)
        {
            CHKERRGL(glDeleteTextures(1, &fc.textureId));
        }
        CHKERRGL(glDeleteBuffers(1, &vboFontVertex));
        CHKERRGL(glDeleteBuffers(1, &vboFontIndex));
        CHKERRGL(glDeleteVertexArrays(1, &vaoFont));
        CHKERRGL(glDeleteProgram(shaderProgram));
    }
    catch(const std::exception &e)
    {
        std::cerr << "GL Error: " << e.what() << '\n';
    }
}
