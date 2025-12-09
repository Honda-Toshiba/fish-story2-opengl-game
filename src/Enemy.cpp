#include "Enemy.h"
#include <cmath>
#include <algorithm>
#include <GLFW/glfw3.h>

Enemy::Enemy(Model* m, glm::vec3 pos, EnemyType t, float s)
    : model(m), position(pos), startPosition(pos), type(t), scale(s), 
      rotationY(0.0f), patrolAngle(0.0f) {
    
    if (type == SHARK) {
        speed = 5.0f;
        patrolRadius = 20.0f;
        collisionRadius = 1.5f * scale;
    } else if (type == HOOK) {
        speed = 2.0f; // Speed of moving up and down
        patrolRadius = 0.0f;
        // Increase collision radius for hooks relative to their small scale
        // Since scale is 0.1, 0.5 * 0.1 = 0.05 which is too small
        // Let's make it larger relative to the visual size
        collisionRadius = 5.0f * scale; 
    } else { // FISH
        speed = 3.0f;
        patrolRadius = 15.0f;
        collisionRadius = 0.0f; // No collision - fish can be eaten
    }
}

void Enemy::Update(float deltaTime, const glm::vec3& playerPos) {
    if (type == SHARK) {
        // Simple patrol logic: circle around start position
        patrolAngle += speed * 0.1f * deltaTime;
        
        float newX = startPosition.x + cos(patrolAngle) * patrolRadius;
        float newZ = startPosition.z + sin(patrolAngle) * patrolRadius;
        
        // Calculate movement direction (where the shark is going)
        glm::vec3 oldPos = position;
        position.x = newX;
        position.z = newZ;
        glm::vec3 movementDirection = position - oldPos;
        
        // Only update rotation if the shark is actually moving
        if (glm::length(movementDirection) > 0.001f) {
            movementDirection = glm::normalize(movementDirection);
            // Face the direction of movement
            // atan2 gives angle from X axis, we adjust for model's forward direction
            rotationY = atan2(movementDirection.x, movementDirection.z) * 180.0f / 3.14159f;
        }
        
        // Bob up and down slightly
        position.y = startPosition.y + sin(patrolAngle * 2.0f) * 2.0f;
    }
    else if (type == FISH) {
        // Fish swim in circles like sharks but smaller
        patrolAngle += speed * 0.15f * deltaTime;
        
        float newX = startPosition.x + cos(patrolAngle) * patrolRadius;
        float newZ = startPosition.z + sin(patrolAngle) * patrolRadius;
        
        // Calculate movement direction for rotation
        glm::vec3 oldPos = position;
        position.x = newX;
        position.z = newZ;
        glm::vec3 movementDirection = position - oldPos;
        
        if (glm::length(movementDirection) > 0.001f) {
            movementDirection = glm::normalize(movementDirection);
            rotationY = atan2(movementDirection.x, movementDirection.z) * 180.0f / 3.14159f;
        }
        
        // Gentle swimming motion
        position.y = startPosition.y + sin(patrolAngle * 3.0f) * 1.5f;
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
    
    if (type == FISH) {
        // Fix fish orientation
        // Try standard model correction: Rotate -90 degrees around X axis
        // This is common for models exported with Z-up (3DS Max) to Y-up (OpenGL)
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        
        // If the fish is still not right, we might need to combine with a Y rotation
        // But let's try this standard fix first.
    }
    
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
