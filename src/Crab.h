#ifndef CRAB_H
#define CRAB_H

#include <glm/glm.hpp>
#include <memory>
#include "Model.h"
#include "Shader.h"

class Crab {
public:
    Crab(Model* model, glm::vec3 position, float patrolRange = 10.0f);
    
    void Update(float deltaTime, const glm::vec3& playerPos);
    void Draw(Shader& shader);
    
    // Returns true if player collides with crab (takes damage)
    bool CheckCollision(const glm::vec3& playerPos, float playerRadius);
    
    bool IsActive() const { return isActive; }
    glm::vec3 GetPosition() const { return position; }
    
private:
    Model* model;
    glm::vec3 position;
    glm::vec3 startPosition;
    float scale;
    float rotationAngle;
    
    bool isActive;
    float collisionRadius;  // Horizontal collision radius
    float collisionHeight;  // Vertical collision height
    
    // Patrol behavior
    float patrolRange;
    float patrolSpeed;
    float patrolDirection; // -1 or 1 for left/right
    float patrolTimer;
    float directionChangeInterval;
    
    // Animation
    float legAnimationTime;
    float legAnimationSpeed;
};

#endif
