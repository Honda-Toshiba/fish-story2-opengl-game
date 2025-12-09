#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <map>
#include <string>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Model.h" // Reusing your texture loading logic

class TextRenderer {
public:
    TextRenderer(unsigned int width, unsigned int height, Shader* shader);
    ~TextRenderer();
    
    // Initialize with a font texture path (e.g., "font.png")
    void Load(std::string fontPath, unsigned int fontSize);
    
    // Draw text string at position (x,y)
    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1.0f));

    void RenderBar(float x, float y, float width, float height, glm::vec3 color);
    
    // Update screen dimensions for proper aspect ratio handling
    void UpdateScreenSize(unsigned int width, unsigned int height);
private:
    unsigned int VAO, VBO;
    unsigned int textureID;
    Shader* shader;
    unsigned int screenWidth, screenHeight;
    
    void initRenderData();
};

#endif