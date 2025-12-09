#include "Collectible.h"
#include <cmath>

Collectible::Collectible(const std::string& modelPath, glm::vec3 pos, float s)
    : position(pos), scale(s), rotationAngle(0.0f), rotationSpeed(50.0f),
      floatOffset(0.0f), floatSpeed(2.0f), initialY(pos.y), isActive(true),
      collisionRadius(2.0f * s) { // Estimate radius based on scale
    
    model = std::make_unique<Model>(modelPath);
}

void Collectible::Update(float deltaTime) {
    if (!isActive) return;

    // Rotate the collectible
    rotationAngle += rotationSpeed * deltaTime;
    if (rotationAngle > 360.0f) rotationAngle -= 360.0f;

    // Floating animation (bobbing up and down)
    floatOffset += floatSpeed * deltaTime;
    position.y = initialY + sin(floatOffset) * 0.5f;
}

void Collectible::Draw(Shader& shader) {
    if (!isActive) return;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));

    shader.setMat4("model", modelMatrix);
    model->Draw(shader);
}

bool Collectible::CheckCollision(const glm::vec3& playerPos, float playerRadius) {
    if (!isActive) return false;

    float distance = glm::length(position - playerPos);
    // Simple sphere collision
    if (distance < (collisionRadius + playerRadius)) {
        return true;
    }
    return false;
}
