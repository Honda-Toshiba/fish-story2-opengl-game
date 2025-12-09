#ifndef ENEMY_H
#define ENEMY_H

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include "Model.h"
#include "Shader.h"

enum EnemyType {
    SHARK,
    HOOK,
    FISH
};

class Enemy {
public:
    Enemy(Model* model, glm::vec3 position, EnemyType type, float scale = 1.0f);
    
    void Update(float deltaTime, const glm::vec3& playerPos);
    void Draw(Shader& shader);
    
    // Returns true if collision occurs
    bool CheckCollision(const glm::vec3& playerPos, float playerRadius);
    
    // Eating animation
    void StartEatingAnimation(const glm::vec3& mouthPos);
    bool IsBeingEaten() const { return beingEaten; }
    bool IsEatenComplete() const { return eatenComplete; }
    
    glm::vec3 GetPosition() const { return position; }
    EnemyType GetType() const { return type; }

private:
    Model* model;
    glm::vec3 position;
    glm::vec3 startPosition;
    glm::vec3 velocity;
    float scale;
    float originalScale;
    float speed;
    float rotationY;
    EnemyType type;
    
    // Patrol logic
    float patrolRadius;
    float patrolAngle;
    
    float collisionRadius;
    
    // Eating animation
    bool beingEaten;
    bool eatenComplete;
    glm::vec3 targetMouthPos;
    float eatAnimationTime;
    float eatAnimationDuration;
};

#endif
