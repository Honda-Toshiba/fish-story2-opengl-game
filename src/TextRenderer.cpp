#include "TextRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h" // Reuse existing stb_image

TextRenderer::TextRenderer(unsigned int width, unsigned int height, Shader* s) 
    : screenWidth(width), screenHeight(height), shader(s), whiteTexID(0) {
    
    // Configure VAO/VBO for texture quads
    initRenderData();
}

TextRenderer::~TextRenderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    if (whiteTexID != 0) {
        glDeleteTextures(1, &whiteTexID);
    }
}

void TextRenderer::Load(std::string fontPath, unsigned int fontSize) {
    // Load Font Texture
    int width, height, nrChannels;
    
    // FIX: Change the last argument from '0' to '4'.
    // This forces stb_image to create an Alpha channel (RGBA) even if the image is just RGB.
    unsigned char *data = stbi_load(fontPath.c_str(), &width, &height, &nrChannels, 4); 
    
    if (data) {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // Set texture options
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Use NEAREST to keep the pixel-art look sharp
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        stbi_image_free(data);
    } else {
        std::cout << "Failed to load font texture: " << fontPath << std::endl;
    }
}

void TextRenderer::RenderText(std::string text, float x, float y, float scale, glm::vec3 color) {
    shader->use();
    glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight);
    shader->setMat4("projection", projection);
    shader->setVec3("textColor", color);
    
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, textureID);

    float charSize = 32.0f * scale; 
    float uvSize = 1.0f / 16.0f; // 16x16 Grid
    
    // --- ADJUSTMENTS FOR 32x32 PIXELS ---
    // Padding: 0.005 is safer for 32px to prevent chipping
    float padding = 0.0f; 
    
    // Spacing: 0.60 fits standard square bitmap fonts best.
    // If gaps are too big, lower this (e.g. 0.5). 
    // If letters overlap, raise it (e.g. 0.7).
    float spacingMultiplier = 0.4f; 
    // ------------------------------------

    for (char& c : text) {
        int ascii = (int)c;
        int col = ascii % 16;
        int row = ascii / 16;
        
        float u_left   = (col * uvSize) + padding;
        float u_right  = ((col + 1) * uvSize) - padding;
        
        float v_top    = (row * uvSize) + padding;
        float v_bottom = ((row + 1) * uvSize) - padding;
        
        float xpos = x;
        float ypos = y;
        float w = charSize;
        float h = charSize;

        float vertices[6][4] = {
            { xpos,     ypos + h,   u_left,     v_top    },            
            { xpos,     ypos,       u_left,     v_bottom },
            { xpos + w, ypos,       u_right,    v_bottom },

            { xpos,     ypos + h,   u_left,     v_top    },
            { xpos + w, ypos,       u_right,    v_bottom },
            { xpos + w, ypos + h,   u_right,    v_top    }           
        };

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Apply spacing
        x += (charSize * spacingMultiplier); 
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::RenderBar(float x, float y, float width, float height, glm::vec3 color) {
    shader->use();
    glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight);
    shader->setMat4("projection", projection);
    shader->setVec3("textColor", color);
    
    // Unbind standard texture so we don't draw letters
    glBindTexture(GL_TEXTURE_2D, 0); 
    
    // NOTE: Your text shader multiplies color * texture. 
    // If texture is unbound (0), it might sample black. 
    // To fix this, we create a 1x1 white texture for this instance.
    if (whiteTexID == 0) {
        glGenTextures(1, &whiteTexID);
        glBindTexture(GL_TEXTURE_2D, whiteTexID);
        unsigned char white[] = { 255, 255, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glBindTexture(GL_TEXTURE_2D, whiteTexID);

    glBindVertexArray(VAO);
    
    float vertices[6][4] = {
        // Pos(x,y)       // UV (Ignored for solid bar)
        { x,         y + height,   0.0f, 0.0f },
        { x,         y,            0.0f, 0.0f },
        { x + width, y,            0.0f, 0.0f },

        { x,         y + height,   0.0f, 0.0f },
        { x + width, y,            0.0f, 0.0f },
        { x + width, y + height,   0.0f, 0.0f }
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::initRenderData() {
    // Configure VAO/VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextRenderer::UpdateScreenSize(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;
}