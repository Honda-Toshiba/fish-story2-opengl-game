#include "Enemy.h"
#include <cmath>
#include <algorithm>
#include <GLFW/glfw3.h>

Enemy::Enemy(const std::string& modelPath, glm::vec3 pos, EnemyType t, float s)
    : position(pos), startPosition(pos), type(t), scale(s), 
      rotationY(0.0f), patrolAngle(0.0f) {
    
    model = std::make_unique<Model>(modelPath);
    
    if (type == SHARK) {
        speed = 5.0f;
        patrolRadius = 20.0f;
        collisionRadius = 1.5f * scale;
    } else { // HOOK
        speed = 2.0f; // Speed of moving up and down
        patrolRadius = 0.0f;
        // Increase collision radius for hooks relative to their small scale
        // Since scale is 0.1, 0.5 * 0.1 = 0.05 which is too small
        // Let's make it larger relative to the visual size
        collisionRadius = 5.0f * scale; 
    }
}

void Enemy::Update(float deltaTime, const glm::vec3& playerPos) {
    if (type == SHARK) {
        // Simple patrol logic: circle around start position
        patrolAngle += speed * 0.1f * deltaTime;
        
        float newX = startPosition.x + cos(patrolAngle) * patrolRadius;
        float newZ = startPosition.z + sin(patrolAngle) * patrolRadius;
        
        // Calculate direction for rotation
        glm::vec3 direction = glm::normalize(glm::vec3(newX, 0.0f, newZ) - position);
        position.x = newX;
        position.z = newZ;
        
        // Face movement direction
        rotationY = -atan2(direction.z, direction.x) * 180.0f / 3.14159f;
        
        // Bob up and down slightly
        position.y = startPosition.y + sin(patrolAngle * 2.0f) * 2.0f;
    } 
    else if (type == HOOK) {
        // Hooks dangle up and down significantly
        // Simulate a fishing line moving up and down
        position.y = startPosition.y + sin(glfwGetTime() * speed) * 5.0f;
        // No rotation for hooks usually, or maybe slight swaying
        rotationY = sin(glfwGetTime()) * 5.0f; 
    }
}

void Enemy::Draw(Shader& shader) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));

    shader.setMat4("model", modelMatrix);
    model->Draw(shader);
}

bool Enemy::CheckCollision(const glm::vec3& playerPos, float playerRadius) {
    float distance = glm::length(position - playerPos);
    if (distance < (collisionRadius + playerRadius)) {
        return true;
    }
    return false;
}
