#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "Model.h"
#include "Shader.h"

enum PowerUpType {
    SPEED_BOOST,      // Green shell - faster movement
    DOUBLE_SCORE      // Yellow shell - 2x score
};

class Collectible {
public:
    Collectible(Model* model, glm::vec3 position, float scale = 1.0f, PowerUpType type = SPEED_BOOST);
    
    void Update(float deltaTime);
    void Draw(Shader& shader);
    
    // Returns true if collision occurs
    bool CheckCollision(const glm::vec3& playerPos, float playerRadius);
    
    bool IsActive() const { return isActive; }
    void Deactivate() { isActive = false; }
    glm::vec3 GetPosition() const { return position; }
    PowerUpType GetPowerUpType() const { return powerUpType; }

private:
    Model* model;
    glm::vec3 position;
    float scale;
    float rotationAngle;
    float rotationSpeed;
    float floatOffset;
    float floatSpeed;
    float initialY;
    PowerUpType powerUpType;
    
    bool isActive;
    float collisionRadius;
};

#endif
