#ifndef CAVE_H
#define CAVE_H

#ifdef _WIN32
    #include <GL/glew.h>
#else
    #include <GL/glew.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "Shader.h"

class Cave {
public:
    Cave(float length, float width, float height);
    ~Cave();
    
    void Update(float deltaTime);
    void Draw(Shader& shader);
    
    float GetLength() const { return length; }
    float GetWidth() const { return width; }
    float GetHeight() const { return height; }
    
    // Check if position is within cave bounds
    bool IsInBounds(const glm::vec3& position) const;
    
    // Get the progress through the cave (0.0 to 1.0)
    float GetProgress(const glm::vec3& position) const;

private:
    float length;   // How long the corridor is (Z-axis)
    float width;    // How wide the corridor is (X-axis)
    float height;   // How tall the corridor is (Y-axis)
    float time;
    
    // OpenGL buffers
    unsigned int floorVAO, floorVBO, floorEBO;
    unsigned int wallLeftVAO, wallLeftVBO, wallLeftEBO;
    unsigned int wallRightVAO, wallRightVBO, wallRightEBO;
    unsigned int ceilingVAO, ceilingVBO, ceilingEBO;
    unsigned int backWallVAO, backWallVBO, backWallEBO;
    unsigned int frontWallVAO, frontWallVBO, frontWallEBO;
    unsigned int cornersVAO, cornersVBO, cornersEBO;
    
    void setupFloor();
    void setupWalls();
    void setupCeiling();
    void setupBackWall();
    void setupFrontWall();
    void setupCorners();
    
    void drawFloor(Shader& shader);
    void drawWalls(Shader& shader);
    void drawCeiling(Shader& shader);
    void drawBackWall(Shader& shader);
    void drawFrontWall(Shader& shader);
    void drawCorners(Shader& shader);
    
    // Helper to create detailed mesh
    void createDetailedPlane(std::vector<float>& vertices, std::vector<unsigned int>& indices,
                            glm::vec3 start, glm::vec3 endX, glm::vec3 endZ, int segmentsX, int segmentsZ);
};

#endif
