//
// Created by cobble on 12/3/2024.
//

#ifndef ITMG_FINAL_SPRITERENDERING_H
#define ITMG_FINAL_SPRITERENDERING_H

#include "shader.hpp"
#include "glm/detail/type_vec2.hpp"
#include "texture2d.hpp"
#include "glm/detail/type_vec3.hpp"

using namespace cobb;
using namespace std;
using namespace glm;

class SpriteRenderer {
public:
    SpriteRenderer(Shader &shader);
    ~SpriteRenderer();

    void DrawSprite(Texture2d &texture, vec2 position, vec2 size = vec2(10.0f, 10.0f), float rotate = 0.0f, vec3 color = vec3(1.0f));
private:
    Shader       shader;
    unsigned int quadVAO;

    void initRenderData();
};


#endif //ITMG_FINAL_SPRITERENDERING_H
