#include "Ocean.h"
#include <vector>
#include <cmath>

Ocean::Ocean(float size, float depth) : size(size), depth(depth), time(0.0f) {
    setupOceanFloor();
    setupWaterSurface();
    setupSkybox();
}

Ocean::~Ocean() {
    glDeleteVertexArrays(1, &oceanFloorVAO);
    glDeleteBuffers(1, &oceanFloorVBO);
    glDeleteBuffers(1, &oceanFloorEBO);
    
    glDeleteVertexArrays(1, &waterVAO);
    glDeleteBuffers(1, &waterVBO);
    glDeleteBuffers(1, &waterEBO);
    
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
}

void Ocean::Update(float deltaTime) {
    time += deltaTime;
}

void Ocean::Draw(Shader& shader) {
    drawSkybox(shader);
    drawOceanFloor(shader);
    drawWaterSurface(shader);
}

void Ocean::setupOceanFloor() {
    // Create a detailed ocean floor with sandy texture coordinates
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    int gridSize = 50;
    float step = (size * 2.0f) / gridSize;
    
    // Generate vertices
    for (int z = 0; z <= gridSize; z++) {
        for (int x = 0; x <= gridSize; x++) {
            float xPos = -size + x * step;
            float zPos = -size + z * step;
            float yPos = -depth + sin(xPos * 0.1f) * 2.0f + cos(zPos * 0.1f) * 2.0f; // Wavy floor
            
            // Position
            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);
            
            // Normal
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
            
            // Texture coords
            vertices.push_back((float)x / gridSize * 10.0f);
            vertices.push_back((float)z / gridSize * 10.0f);
        }
    }
    
    // Generate indices
    for (int z = 0; z < gridSize; z++) {
        for (int x = 0; x < gridSize; x++) {
            int topLeft = z * (gridSize + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (gridSize + 1) + x;
            int bottomRight = bottomLeft + 1;
            
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    
    glGenVertexArrays(1, &oceanFloorVAO);
    glGenBuffers(1, &oceanFloorVBO);
    glGenBuffers(1, &oceanFloorEBO);
    
    glBindVertexArray(oceanFloorVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, oceanFloorVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oceanFloorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void Ocean::setupWaterSurface() {
    // Create invisible water surface boundaries (for reference)
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    int gridSize = 20;
    float step = (size * 2.0f) / gridSize;
    
    for (int z = 0; z <= gridSize; z++) {
        for (int x = 0; x <= gridSize; x++) {
            float xPos = -size + x * step;
            float zPos = -size + z * step;
            
            vertices.push_back(xPos);
            vertices.push_back(10.0f); // Water surface height
            vertices.push_back(zPos);
            
            vertices.push_back(0.0f);
            vertices.push_back(-1.0f);
            vertices.push_back(0.0f);
            
            vertices.push_back((float)x / gridSize);
            vertices.push_back((float)z / gridSize);
        }
    }
    
    for (int z = 0; z < gridSize; z++) {
        for (int x = 0; x < gridSize; x++) {
            int topLeft = z * (gridSize + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (gridSize + 1) + x;
            int bottomRight = bottomLeft + 1;
            
            indices.push_back(topLeft);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            
            indices.push_back(topRight);
            indices.push_back(bottomRight);
            indices.push_back(bottomLeft);
        }
    }
    
    glGenVertexArrays(1, &waterVAO);
    glGenBuffers(1, &waterVBO);
    glGenBuffers(1, &waterEBO);
    
    glBindVertexArray(waterVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void Ocean::setupSkybox() {
    // Simple skybox cube
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void Ocean::drawOceanFloor(Shader& shader) {
    shader.setVec3("objectColor", 0.76f, 0.70f, 0.50f); // Sandy color
    shader.setBool("isFloor", true);
    
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    
    glBindVertexArray(oceanFloorVAO);
    glDrawElements(GL_TRIANGLES, 50 * 50 * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    shader.setBool("isFloor", false);
}

void Ocean::drawWaterSurface(Shader& shader) {
    // Draw water surface with transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    shader.setVec3("objectColor", 0.0f, 0.4f, 0.7f);
    shader.setFloat("waterTime", time);
    shader.setBool("isWater", true);
    
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    
    glBindVertexArray(waterVAO);
    glDrawElements(GL_TRIANGLES, 20 * 20 * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    shader.setBool("isWater", false);
    glDisable(GL_BLEND);
}

void Ocean::drawSkybox(Shader& shader) {
    // Draw skybox with gradient (underwater atmosphere)
    glDepthFunc(GL_LEQUAL);
    
    shader.setVec3("skyColor", 0.1f, 0.3f, 0.5f); // Deep blue underwater
    shader.setBool("isSkybox", true);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(size * 2.0f));
    shader.setMat4("model", model);
    
    glBindVertexArray(skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    shader.setBool("isSkybox", false);
    glDepthFunc(GL_LESS);
}
