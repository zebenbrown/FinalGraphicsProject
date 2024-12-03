//
// Created by Zeben on 11/22/2024.
//

#ifndef ITMG_FINAL_TEXTRENDERING_H
#define ITMG_FINAL_TEXTRENDERING_H


#include <ft2build.h>
#include FT_FREETYPE_H

#include "../ew/external/glad.h"
#include "shader.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <map>


class textRendering {
public:

    struct Character {
        unsigned int textureID; //ID handle of glyph texture
        glm::ivec2 Size; //Size of glyph
        glm::ivec2 Bearing; //Offset from baseline to top/left of glyph
        unsigned int Advance; //Horizontal offset to advance to next glyph
    };

    textRendering()
    {
        //configure vao and vbo
        glGenVertexArrays(1, &textVAO);
        glGenBuffers(1, &textVBO);
        glBindVertexArray(textVAO);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void RenderText(cobb::Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color);

    float getWidth(string text, float scale);
    float getHeight(float scale);

    FT_Library ft;
    FT_Face face;
    std::map<GLchar, Character> characterMap;

    void loadText(std::string fontPath);

    ~textRendering()
    {
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

private:
    unsigned int textVAO, textVBO;

#endif //ITMG_FINAL_TEXTRENDERING_H
};