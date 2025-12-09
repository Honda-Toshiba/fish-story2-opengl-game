#include "Stalactite.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Stalactite::Stalactite(Model* model, const glm::vec3& spawnPos, float hangTime)
    : model(model), position(spawnPos), active(true), 
      state(HANGING), hangTimer(0.0f), hangDuration(hangTime),
      fallSpeed(0.0f), scale(2.0f), collisionRadius(1.0f), collisionHeight(3.0f) {
}

void Stalactite::Update(float deltaTime) {
    if (!active) return;
    
    switch (state) {
        case HANGING:
            hangTimer += deltaTime;
            if (hangTimer >= hangDuration) {
                state = FALLING;
                hangTimer = 0.0f;
            }
            break;
            
        case FALLING:
            // Accelerate downward (gravity)
            fallSpeed += 15.0f * deltaTime;  // Gravity acceleration
            position.y -= fallSpeed * deltaTime;
            
            // Check if stalactite hit the floor (cave floor is at y = 0)
            if (position.y <= 0.0f) {
                active = false;
                state = INACTIVE;
            }
            break;
            
        case INACTIVE:
            active = false;
            break;
    }
}

void Stalactite::Draw(Shader& shader) {
    if (!active) return;
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Flip upside down
    model = glm::scale(model, glm::vec3(scale));
    
    shader.setMat4("model", model);
    shader.setBool("hasTexture", true);
    shader.setBool("isFloor", false);
    shader.setBool("isWater", false);
    shader.setBool("isSkybox", false);
    shader.setBool("isSun", false);
    shader.setBool("isGlowingFish", false);
    shader.setBool("isPowerUp", false);
    shader.setBool("isCave", false);
    shader.setBool("isGlowing", false);
    
    this->model->Draw(shader);
}

bool Stalactite::CheckCollision(const glm::vec3& point, float radius) const {
    if (!active) return false;
    
    // Cylindrical collision detection
    float dx = point.x - position.x;
    float dz = point.z - position.z;
    float horizontalDist = sqrt(dx * dx + dz * dz);
    
    // Check if within horizontal radius
    if (horizontalDist > (collisionRadius + radius)) {
        return false;
    }
    
    // Check if within vertical range (stalactite hangs down from ceiling)
    float topY = position.y + collisionHeight;
    float bottomY = position.y - collisionHeight;
    
    if (point.y < bottomY || point.y > topY) {
        return false;
    }
    
    return true;
}
