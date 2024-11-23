//
// Created by Zeben on 11/22/2024.
//

#ifndef ITMG_FINAL_TEXTRENDERING_H
#define ITMG_FINAL_TEXTRENDERING_H


#include <ft2build.h>
#include FT_FREETYPE_H

#include "../ew/external/glad.h"
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

    FT_Library ft;
    FT_Face face;
    std::map<GLchar, Character> characterMap;

    void loadText(std::string fontPath);

    ~textRendering()
    {
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

#endif //ITMG_FINAL_TEXTRENDERING_H
};