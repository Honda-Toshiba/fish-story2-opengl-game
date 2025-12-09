#ifndef STALACTITE_H
#define STALACTITE_H

#include <glm/glm.hpp>
#include "Model.h"

class Stalactite {
public:
    Stalactite(Model* model, const glm::vec3& spawnPos, float hangTime = 2.0f);
    
    void Update(float deltaTime);
    void Draw(Shader& shader);
    
    bool IsActive() const { return active; }
    void Deactivate() { active = false; }
    
    // Check collision with a sphere (player)
    bool CheckCollision(const glm::vec3& point, float radius) const;
    
    glm::vec3 position;
    
private:
    Model* model;
    bool active;
    
    // State machine
    enum State {
        HANGING,
        FALLING,
        INACTIVE
    };
    State state;
    
    float hangTimer;
    float hangDuration;
    float fallSpeed;
    float scale;
    
    // Collision
    float collisionRadius;
    float collisionHeight;
};

#endif
