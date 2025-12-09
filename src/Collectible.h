#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "Model.h"
#include "Shader.h"

class Collectible {
public:
    Collectible(Model* model, glm::vec3 position, float scale = 1.0f);
    
    void Update(float deltaTime);
    void Draw(Shader& shader);
    
    // Returns true if collision occurs
    bool CheckCollision(const glm::vec3& playerPos, float playerRadius);
    
    bool IsActive() const { return isActive; }
    void Deactivate() { isActive = false; }
    glm::vec3 GetPosition() const { return position; }

private:
    Model* model;
    glm::vec3 position;
    float scale;
    float rotationAngle;
    float rotationSpeed;
    float floatOffset;
    float floatSpeed;
    float initialY;
    
    bool isActive;
    float collisionRadius;
};

#endif
