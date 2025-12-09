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
    Enemy(const std::string& modelPath, glm::vec3 position, EnemyType type, float scale = 1.0f);
    
    void Update(float deltaTime, const glm::vec3& playerPos);
    void Draw(Shader& shader);
    
    // Returns true if collision occurs
    bool CheckCollision(const glm::vec3& playerPos, float playerRadius);
    
    glm::vec3 GetPosition() const { return position; }

private:
    std::unique_ptr<Model> model;
    glm::vec3 position;
    glm::vec3 startPosition;
    glm::vec3 velocity;
    float scale;
    float speed;
    float rotationY;
    EnemyType type;
    
    // Patrol logic
    float patrolRadius;
    float patrolAngle;
    
    float collisionRadius;
};

#endif
