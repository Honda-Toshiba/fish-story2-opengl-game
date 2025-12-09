#include "Crab.h"
#include <cmath>
#include <iostream>

Crab::Crab(Model* m, glm::vec3 pos, float range)
    : model(m), position(pos), startPosition(pos), scale(0.1f),  // Much smaller
      rotationAngle(0.0f),
      isActive(true),
      collisionRadius(1.5f),  // Horizontal collision radius
      collisionHeight(4.0f),  // Vertical collision height for more challenge
      patrolRange(range),
      patrolSpeed(5.0f),      // Increased from 3.5 to 5.0 for even faster movement
      patrolTimer(0.0f),
      directionChangeInterval(3.0f),
      legAnimationTime(0.0f),
      legAnimationSpeed(5.0f) {
    
    // Set initial direction based on starting position
    // If starting on the right (positive X), move left (negative direction)
    // If starting on the left (negative X), move right (positive direction)
    if (pos.x > 0) {
        patrolDirection = -1.0f; // Start moving left
        rotationAngle = -90.0f;
    } else {
        patrolDirection = 1.0f; // Start moving right
        rotationAngle = 90.0f;
    }
}

void Crab::Update(float deltaTime, const glm::vec3& playerPos) {
    if (!isActive) return;
    
    // Move along X axis (sideways like a crab)
    position.x += patrolDirection * patrolSpeed * deltaTime;
    
    // Determine boundaries based on start position
    // If starting on the right (positive X), move left (negative direction)
    // If starting on the left (negative X), move right (positive direction)
    float leftBoundary, rightBoundary;
    if (startPosition.x > 0) {
        // Started on right wall, patrol to the left
        rightBoundary = startPosition.x;
        leftBoundary = startPosition.x - patrolRange;
    } else {
        // Started on left wall, patrol to the right
        leftBoundary = startPosition.x;
        rightBoundary = startPosition.x + patrolRange;
    }
    
    // Check boundaries and reverse direction when hit
    if (position.x > rightBoundary) {
        position.x = rightBoundary;
        patrolDirection = -1.0f;
        rotationAngle = -90.0f;
    } else if (position.x < leftBoundary) {
        position.x = leftBoundary;
        patrolDirection = 1.0f;
        rotationAngle = 90.0f;
    }
    
    // Update leg animation
    legAnimationTime += deltaTime * legAnimationSpeed;
}

void Crab::Draw(Shader& shader) {
    if (!isActive) return;
    
    shader.setBool("isGlowing", false);
    shader.setBool("isCave", false);
    
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, position);
    modelMat = glm::rotate(modelMat, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMat = glm::scale(modelMat, glm::vec3(scale));
    
    shader.setMat4("model", modelMat);
    model->Draw(shader);
}

bool Crab::CheckCollision(const glm::vec3& playerPos, float playerRadius) {
    if (!isActive) return false;
    
    // Use cylindrical collision: check horizontal distance and vertical distance separately
    // This makes crabs more dangerous vertically
    float horizontalDist = glm::length(glm::vec2(playerPos.x - position.x, playerPos.z - position.z));
    float verticalDist = std::abs(playerPos.y - position.y);
    
    // Check if within horizontal radius and vertical height
    return (horizontalDist < (collisionRadius + playerRadius)) && (verticalDist < collisionHeight);
}
