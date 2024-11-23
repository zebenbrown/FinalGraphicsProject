//
// Created by Zeben on 11/22/2024.
//

#include "textRendering.h"


void textRendering::loadText(std::string fontPath)
{
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Failed tp initialize freetype" << std::endl;
    }

    //find path to font
    if (fontPath.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font name" << std::endl;
    }

    //load font as face
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }
    else
    {
        //set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        //disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        //load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            //load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYPE: Failed to load glyph" << std::endl;
                continue;
            }

            //generate texture
            unsigned int glyphTexture;
            glGenTextures(1, &glyphTexture);
            glBindTexture(GL_TEXTURE_2D, glyphTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

            //set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            //store character for later use
            Character character =
                    {
                    glyphTexture, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), static_cast<unsigned int>(face->glyph->advance.x)
                    };
            characterMap.insert(std::pair<char, Character>(c, character));
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
