#ifndef ANGLERFISH_H
#define ANGLERFISH_H

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "Model.h"
#include "Shader.h"

class Anglerfish {
public:
    Anglerfish(Model* model, glm::vec3 position, float scale = 0.5f);
    
    void Update(float deltaTime, const glm::vec3& playerPos);
    void Draw(Shader& shader);
    
    // Returns true if collision occurs
    bool CheckCollision(const glm::vec3& playerPos, float playerRadius);
    
    bool IsActive() const { return isActive; }
    bool IsCollected() const { return isCollected; }
    void Collect();
    
    glm::vec3 GetPosition() const { return position; }
    glm::vec3 GetLightColor() const { return lightColor; }
    
    // Return light position offset in front of the fish (not at center)
    glm::vec3 GetLightPosition() const { 
        // Position light in front of the fish based on its rotation
        float angleRad = glm::radians(rotationAngle);
        float offsetX = sin(angleRad) * 3.0f;  // 3 units in front
        float offsetZ = cos(angleRad) * 3.0f;
        return position + glm::vec3(offsetX, 0.5f, offsetZ); 
    }
    
    float GetLightIntensity() const { 
        // Only provide light when collected, and it dims as the fish shrinks
        if (!isCollected) return 0.0f; // No light until collected!
        return lightIntensity;
    }
    float GetLightRadius() const { return lightRadius; }
    
    // Only collected Anglerfish provide light and follow the player
    bool ProvidesLight() const { return isCollected; }

private:
    Model* model;
    glm::vec3 position;
    glm::vec3 startPosition;
    float scale;
    float rotationAngle;
    float rotationSpeed;
    float floatOffset;
    float floatSpeed;
    float initialY;
    
    bool isActive;
    bool isCollected;
    float collisionRadius;
    
    // Light properties
    glm::vec3 lightColor;      // Blueish-green bioluminescent color
    float lightIntensity;       // Brightness of the light
    float lightRadius;          // How far the light reaches
    
    // Following behavior
    glm::vec3 targetPosition;
    float followSpeed;
    float followDistance;      // Distance to maintain from player
    float orbitAngle;          // Angle for orbiting around player
    float shrinkRate;          // How fast the fish shrinks after collection
    float currentScale;        // Current scale (shrinks over time when collected)
};

#endif
