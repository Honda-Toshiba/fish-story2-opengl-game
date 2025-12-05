#ifndef OCEAN_H
#define OCEAN_H

#ifdef _WIN32
    #include <GL/glew.h>
#else
    #include <GL/glew.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"

class Ocean {
public:
    Ocean(float size, float depth);
    ~Ocean();
    
    void Update(float deltaTime);
    void Draw(Shader& shader);
    
    float GetSize() const { return size; }
    float GetDepth() const { return depth; }

private:
    float size;
    float depth;
    float time;
    
    unsigned int oceanFloorVAO, oceanFloorVBO, oceanFloorEBO;
    unsigned int waterVAO, waterVBO, waterEBO;
    unsigned int skyboxVAO, skyboxVBO;
    
    void setupOceanFloor();
    void setupWaterSurface();
    void setupSkybox();
    
    void drawOceanFloor(Shader& shader);
    void drawWaterSurface(Shader& shader);
    void drawSkybox(Shader& shader);
};

#endif
