#include "Anglerfish.h"
#include <cmath>
#include <iostream>

Anglerfish::Anglerfish(Model* m, glm::vec3 pos, float s)
    : model(m), position(pos), startPosition(pos), scale(s), 
      rotationAngle(0.0f), rotationSpeed(30.0f),
      floatOffset(0.0f), floatSpeed(1.5f), initialY(pos.y), 
      isActive(true), isCollected(false),
      collisionRadius(5.0f),  // Fixed collision radius (not based on visual scale)
      lightColor(1.0f, 0.7f, 0.3f),  // Soft yellow-orange color
      lightIntensity(8.0f),           // MASSIVELY increased brightness (was 3.0)
      lightRadius(20.0f),             // Larger radius for more coverage
      followSpeed(5.0f),
      followDistance(3.0f),
      orbitAngle(0.0f),
      shrinkRate(5.0f),               // Shrink by 5 units per second
      currentScale(s) {                // Start at full scale
}

void Anglerfish::Update(float deltaTime, const glm::vec3& playerPos) {
    if (!isActive) return;
    
    if (!isCollected) {
        // Not collected yet - float in place with gentle movement
        // Don't rotate - just face forward (rotationAngle stays at initial value)
        
        // Floating animation (bobbing up and down)
        floatOffset += floatSpeed * deltaTime;
        position.y = initialY + sin(floatOffset) * 0.3f;
        
        // Gentle swaying side to side
        position.x = startPosition.x + sin(floatOffset * 0.7f) * 0.5f;
    } else {
        // Collected - follow player and orbit around them
        orbitAngle += rotationSpeed * 0.5f * deltaTime;
        if (orbitAngle > 360.0f) orbitAngle -= 360.0f;
        
        // Calculate orbit position around player
        float orbitX = cos(glm::radians(orbitAngle)) * followDistance;
        float orbitY = sin(floatOffset) * 0.5f; // Gentle bobbing
        float orbitZ = sin(glm::radians(orbitAngle)) * followDistance;
        
        targetPosition = playerPos + glm::vec3(orbitX, orbitY + 1.0f, orbitZ);
        
        // Smoothly move towards target position
        glm::vec3 direction = targetPosition - position;
        float distance = glm::length(direction);
        
        if (distance > 0.1f) {
            direction = glm::normalize(direction);
            position += direction * followSpeed * deltaTime;
        }
        
        // Update float offset for bobbing
        floatOffset += floatSpeed * deltaTime;
        
        // Face the player
        glm::vec3 toPlayer = playerPos - position;
        if (glm::length(toPlayer) > 0.01f) {
            toPlayer = glm::normalize(toPlayer);
            rotationAngle = atan2(toPlayer.x, toPlayer.z) * 180.0f / 3.14159f;
        }
        
        // Gradually shrink the fish until it disappears (consumed by player)
        currentScale -= shrinkRate * deltaTime;
        
        // Also dim the light as fish shrinks
        float shrinkPercent = currentScale / scale; // 1.0 at start, 0.0 when gone
        lightIntensity = 8.0f * shrinkPercent;
        
        // Deactivate when too small
        if (currentScale <= 0.0f) {
            isActive = false;
            currentScale = 0.0f;
        }
    }
}

void Anglerfish::Draw(Shader& shader) {
    if (!isActive) return;
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    
    // First rotate around Y axis for horizontal direction
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Then pitch down -90 degrees so fish looks forward (was +90, made them upside down)
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    
    // Use currentScale which shrinks over time when collected
    modelMatrix = glm::scale(modelMatrix, glm::vec3(currentScale));
    
    shader.setMat4("model", modelMatrix);
    
    // Render the model naturally without any glow override
    // Just set neutral color and let the texture show through
    shader.setVec3("objectColor", 1.0f, 1.0f, 1.0f); // Pure white - no tint
    shader.setBool("isGlowing", false); // No emissive glow on the model itself
    shader.setFloat("glowIntensity", 0.0f);
    
    model->Draw(shader); // Mesh will set hasTexture=true internally if textures exist
    
    shader.setBool("isGlowing", false);
}

bool Anglerfish::CheckCollision(const glm::vec3& playerPos, float playerRadius) {
    if (!isActive || isCollected) return false;
    
    float distance = glm::length(position - playerPos);
    if (distance < (collisionRadius + playerRadius)) {
        return true;
    }
    return false;
}

void Anglerfish::Collect() {
    if (!isCollected) {
        isCollected = true;
        std::cout << "Anglerfish collected! The cave gets brighter..." << std::endl;
    }
}
