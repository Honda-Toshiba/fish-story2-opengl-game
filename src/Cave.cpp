#include "Cave.h"
#include <vector>
#include <cmath>
#include <iostream>

Cave::Cave(float len, float w, float h) 
    : length(len), width(w), height(h), time(0.0f) {
    
    std::cout << "Creating cave: " << length << "m long, " 
              << width << "m wide, " << height << "m tall" << std::endl;
    
    setupFloor();
    setupWalls();
    setupCeiling();
    setupBackWall();
    setupFrontWall();
    setupCorners();
}

Cave::~Cave() {
    glDeleteVertexArrays(1, &floorVAO);
    glDeleteBuffers(1, &floorVBO);
    glDeleteBuffers(1, &floorEBO);
    
    glDeleteVertexArrays(1, &wallLeftVAO);
    glDeleteBuffers(1, &wallLeftVBO);
    glDeleteBuffers(1, &wallLeftEBO);
    
    glDeleteVertexArrays(1, &wallRightVAO);
    glDeleteBuffers(1, &wallRightVBO);
    glDeleteBuffers(1, &wallRightEBO);
    
    glDeleteVertexArrays(1, &ceilingVAO);
    glDeleteBuffers(1, &ceilingVBO);
    glDeleteBuffers(1, &ceilingEBO);
    
    glDeleteVertexArrays(1, &backWallVAO);
    glDeleteBuffers(1, &backWallVBO);
    glDeleteBuffers(1, &backWallEBO);
    
    glDeleteVertexArrays(1, &frontWallVAO);
    glDeleteBuffers(1, &frontWallVBO);
    glDeleteBuffers(1, &frontWallEBO);
    
    glDeleteVertexArrays(1, &cornersVAO);
    glDeleteBuffers(1, &cornersVBO);
    glDeleteBuffers(1, &cornersEBO);
}

void Cave::Update(float deltaTime) {
    time += deltaTime;
}

void Cave::Draw(Shader& shader) {
    shader.setBool("isCave", true);
    shader.setBool("hasTexture", false);
    shader.setBool("isWater", false);  // Make sure water animation is off
    
    // Disable backface culling for cave to ensure all faces visible
    glDisable(GL_CULL_FACE);
    
    drawFloor(shader);
    drawWalls(shader);
    drawCeiling(shader);
    drawBackWall(shader);
    drawFrontWall(shader);
    // Corners removed - floor/ceiling now extend to cover gaps
    
    // Re-enable backface culling
    glEnable(GL_CULL_FACE);
    
    shader.setBool("isCave", false);
}

bool Cave::IsInBounds(const glm::vec3& position) const {
    return position.x >= -width/2 && position.x <= width/2 &&
           position.y >= 0.0f && position.y <= height &&
           position.z >= 0.0f && position.z <= length;
}

float Cave::GetProgress(const glm::vec3& position) const {
    return glm::clamp(position.z / length, 0.0f, 1.0f);
}

void Cave::createDetailedPlane(std::vector<float>& vertices, std::vector<unsigned int>& indices,
                               glm::vec3 start, glm::vec3 endX, glm::vec3 endZ, 
                               int segmentsX, int segmentsZ) {
    vertices.clear();
    indices.clear();
    
    glm::vec3 normal = glm::normalize(glm::cross(endX - start, endZ - start));
    
    // Generate vertices
    for (int z = 0; z <= segmentsZ; z++) {
        for (int x = 0; x <= segmentsX; x++) {
            float u = (float)x / segmentsX;
            float v = (float)z / segmentsZ;
            
            glm::vec3 pos = start + (endX - start) * u + (endZ - start) * v;
            
            // Add some rocky variation
            float noise = sin(pos.x * 0.5f) * cos(pos.z * 0.5f) * 0.3f;
            pos += normal * noise;
            
            // Position
            vertices.push_back(pos.x);
            vertices.push_back(pos.y);
            vertices.push_back(pos.z);
            
            // Normal
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
            
            // Texture coords
            vertices.push_back(u * 10.0f);
            vertices.push_back(v * 10.0f);
        }
    }
    
    // Generate indices
    for (int z = 0; z < segmentsZ; z++) {
        for (int x = 0; x < segmentsX; x++) {
            int topLeft = z * (segmentsX + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (segmentsX + 1) + x;
            int bottomRight = bottomLeft + 1;
            
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
}

void Cave::setupFloor() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Floor plane - extend beyond walls to cover corner gaps
    // Order: start, then Z direction, then X direction for upward normal
    float extension = 1.0f;
    glm::vec3 start(-width/2 - extension, 0.0f, -extension);
    glm::vec3 endX(width/2 + extension, 0.0f, -extension);            // Along X
    glm::vec3 endZ(-width/2 - extension, 0.0f, length + extension);  // Along Z
    
    // Pass in correct order: start, endZ (along Z first), endX (along X second)
    // This makes cross(endZ-start, endX-start) point upward
    createDetailedPlane(vertices, indices, start, endZ, endX, 40, 80);
    
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glGenBuffers(1, &floorEBO);
    
    glBindVertexArray(floorVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Texture coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void Cave::setupWalls() {
    // Left Wall - order vectors so normal faces inward (to the right)
    {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        
        glm::vec3 start(-width/2, 0.0f, 0.0f);
        glm::vec3 endX(-width/2, height, 0.0f);  // Up
        glm::vec3 endZ(-width/2, 0.0f, length);  // Forward
        
        createDetailedPlane(vertices, indices, start, endX, endZ, 30, 80);
        
        glGenVertexArrays(1, &wallLeftVAO);
        glGenBuffers(1, &wallLeftVBO);
        glGenBuffers(1, &wallLeftEBO);
        
        glBindVertexArray(wallLeftVAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, wallLeftVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallLeftEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        
        glBindVertexArray(0);
    }
    
    // Right Wall - swap order to face inward (negative X direction)
    {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        
        glm::vec3 start(width/2, 0.0f, 0.0f);
        glm::vec3 endZ(width/2, 0.0f, length);   // Forward
        glm::vec3 endX(width/2, height, 0.0f);   // Up
        
        createDetailedPlane(vertices, indices, start, endZ, endX, 30, 80);
        
        glGenVertexArrays(1, &wallRightVAO);
        glGenBuffers(1, &wallRightVBO);
        glGenBuffers(1, &wallRightEBO);
        
        glBindVertexArray(wallRightVAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, wallRightVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallRightEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        
        glBindVertexArray(0);
    }
}

void Cave::setupCeiling() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Ceiling - extend beyond walls to cover corner gaps
    float extension = 1.0f;
    glm::vec3 start(-width/2 - extension, height, -extension);
    glm::vec3 endX(width/2 + extension, height, -extension);
    glm::vec3 endZ(-width/2 - extension, height, length + extension);
    
    createDetailedPlane(vertices, indices, start, endX, endZ, 40, 80);
    
    glGenVertexArrays(1, &ceilingVAO);
    glGenBuffers(1, &ceilingVBO);
    glGenBuffers(1, &ceilingEBO);
    
    glBindVertexArray(ceilingVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, ceilingVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ceilingEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void Cave::setupBackWall() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Back wall at the end of the cave - extend to overlap with floor/ceiling
    float extension = 1.0f;
    glm::vec3 start(-width/2 - extension, -extension, length);
    glm::vec3 endX(width/2 + extension, -extension, length);
    glm::vec3 endZ(-width/2 - extension, height + extension, length);
    
    createDetailedPlane(vertices, indices, start, endX, endZ, 20, 20);
    
    glGenVertexArrays(1, &backWallVAO);
    glGenBuffers(1, &backWallVBO);
    glGenBuffers(1, &backWallEBO);
    
    glBindVertexArray(backWallVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, backWallVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backWallEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void Cave::drawFloor(Shader& shader) {
    shader.setVec3("objectColor", 0.35f, 0.32f, 0.30f); // Lighter rocky color to reflect light better
    shader.setBool("isFloor", true);
    
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    
    glBindVertexArray(floorVAO);
    glDrawElements(GL_TRIANGLES, 40 * 80 * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    shader.setBool("isFloor", false);
}

void Cave::drawWalls(Shader& shader) {
    shader.setVec3("objectColor", 0.40f, 0.38f, 0.35f); // Much lighter walls to reflect anglerfish light
    
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    
    // Left wall
    glBindVertexArray(wallLeftVAO);
    glDrawElements(GL_TRIANGLES, 30 * 80 * 6, GL_UNSIGNED_INT, 0);
    
    // Right wall
    glBindVertexArray(wallRightVAO);
    glDrawElements(GL_TRIANGLES, 30 * 80 * 6, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
}

void Cave::drawCeiling(Shader& shader) {
    shader.setVec3("objectColor", 0.30f, 0.28f, 0.26f); // Lighter ceiling to reflect light
    
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    
    glBindVertexArray(ceilingVAO);
    glDrawElements(GL_TRIANGLES, 40 * 80 * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Cave::drawBackWall(Shader& shader) {
    shader.setVec3("objectColor", 0.40f, 0.38f, 0.35f); // Match side walls
    
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    
    glBindVertexArray(backWallVAO);
    glDrawElements(GL_TRIANGLES, 20 * 20 * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Cave::setupFrontWall() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Front wall at entrance (Z=0) - extend to overlap with floor/ceiling
    float extension = 1.0f;
    glm::vec3 start(-width/2 - extension, -extension, 0.0f);
    glm::vec3 endZ(-width/2 - extension, height + extension, 0.0f);  // Up
    glm::vec3 endX(width/2 + extension, -extension, 0.0f);     // Right
    
    createDetailedPlane(vertices, indices, start, endX, endZ, 20, 20);
    
    glGenVertexArrays(1, &frontWallVAO);
    glGenBuffers(1, &frontWallVBO);
    glGenBuffers(1, &frontWallEBO);
    
    glBindVertexArray(frontWallVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, frontWallVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frontWallEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void Cave::setupCorners() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Create thick boxes to fill corner gaps - need to overlap with main geometry
    float cornerThickness = 1.5f;  // Much thicker to ensure overlap
    
    // Helper lambda to add a box
    auto addBox = [&](glm::vec3 min, glm::vec3 max) {
        int startIdx = vertices.size() / 8;
        
        // 8 corners of box
        glm::vec3 corners[8] = {
            {min.x, min.y, min.z}, {max.x, min.y, min.z},
            {max.x, max.y, min.z}, {min.x, max.y, min.z},
            {min.x, min.y, max.z}, {max.x, min.y, max.z},
            {max.x, max.y, max.z}, {min.x, max.y, max.z}
        };
        
        // Add vertices with simple normals
        for (int i = 0; i < 8; i++) {
            vertices.push_back(corners[i].x);
            vertices.push_back(corners[i].y);
            vertices.push_back(corners[i].z);
            vertices.push_back(0.0f); // Normal X
            vertices.push_back(1.0f); // Normal Y
            vertices.push_back(0.0f); // Normal Z
            vertices.push_back(0.0f); // TexCoord U
            vertices.push_back(0.0f); // TexCoord V
        }
        
        // Add faces (12 triangles for box)
        int faces[12][3] = {
            {0,1,2}, {0,2,3},   // Front
            {4,6,5}, {4,7,6},   // Back
            {0,3,7}, {0,7,4},   // Left
            {1,5,6}, {1,6,2},   // Right
            {3,2,6}, {3,6,7},   // Top
            {0,4,5}, {0,5,1}    // Bottom
        };
        
        for (int i = 0; i < 12; i++) {
            indices.push_back(startIdx + faces[i][0]);
            indices.push_back(startIdx + faces[i][1]);
            indices.push_back(startIdx + faces[i][2]);
        }
    };
    
    // 4 vertical corners (full height) - extend beyond edges to overlap
    float w2 = width/2;
    addBox({-w2 - cornerThickness, -cornerThickness, -cornerThickness}, {-w2 + cornerThickness, height + cornerThickness, cornerThickness}); // Front-left
    addBox({w2 - cornerThickness, -cornerThickness, -cornerThickness}, {w2 + cornerThickness, height + cornerThickness, cornerThickness});   // Front-right
    addBox({-w2 - cornerThickness, -cornerThickness, length - cornerThickness}, {-w2 + cornerThickness, height + cornerThickness, length + cornerThickness}); // Back-left
    addBox({w2 - cornerThickness, -cornerThickness, length - cornerThickness}, {w2 + cornerThickness, height + cornerThickness, length + cornerThickness});   // Back-right
    
    // Top-bottom edge fillers (along length) - extend to overlap with vertical corners
    addBox({-w2 - cornerThickness, -cornerThickness, 0}, {-w2 + cornerThickness, cornerThickness, length});           // Bottom-left edge
    addBox({w2 - cornerThickness, -cornerThickness, 0}, {w2 + cornerThickness, cornerThickness, length});             // Bottom-right edge
    addBox({-w2 - cornerThickness, height - cornerThickness, 0}, {-w2 + cornerThickness, height + cornerThickness, length}); // Top-left edge
    addBox({w2 - cornerThickness, height - cornerThickness, 0}, {w2 + cornerThickness, height + cornerThickness, length});   // Top-right edge
    
    glGenVertexArrays(1, &cornersVAO);
    glGenBuffers(1, &cornersVBO);
    glGenBuffers(1, &cornersEBO);
    
    glBindVertexArray(cornersVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, cornersVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cornersEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void Cave::drawFrontWall(Shader& shader) {
    shader.setVec3("objectColor", 0.40f, 0.38f, 0.35f); // Match other walls
    
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    
    glBindVertexArray(frontWallVAO);
    glDrawElements(GL_TRIANGLES, 20 * 20 * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Cave::drawCorners(Shader& shader) {
    shader.setVec3("objectColor", 0.35f, 0.33f, 0.30f); // Lighter corners
    
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    
    glBindVertexArray(cornersVAO);
    glDrawElements(GL_TRIANGLES, 8 * 12 * 3, GL_UNSIGNED_INT, 0); // 8 boxes × 12 triangles × 3 indices
    glBindVertexArray(0);
}
