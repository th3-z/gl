#ifndef GL_FONT_H
#define GL_FONT_H

#include "graphics/shader.h"
#include "graphics/texture.h"
#include <iostream>
#include <vector>

class Font {
public:
    explicit Font(const char *str);
    void draw();

private:
    unsigned int textureHandle;
    unsigned int VAO;
    std::vector<unsigned int> VBOs;
    Shader *shader;
    Texture *texture;

    std::vector<float> vertices;
    std::vector<float> uvs;
    std::vector<int> indices;

    std::pair<float, float> getTexelCoord(int x, int y);
};

#endif // GL_FONT_H
