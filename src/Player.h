#ifndef PLAYER_H
#define PLAYER_H

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Model.h"
#include "Shader.h"

class Player {
public:
    // Player state
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    
    float yaw;
    float pitch;
    float scale;
    float speed;
    float sprintMultiplier;
    bool isSprinting;          // Is the player currently boosting?
    float sprintTimer;         // How long have we been boosting?
    float sprintDuration;      // Max time we can boost (e.g. 2.0s)
    float sprintCooldown;      // How long to wait after boosting (e.g. 5.0s)
    float sprintCooldownTimer; // Tracks the waiting time
    bool isBoostActive;
    float boostMultiplier;
    float boostDuration;
    float boostCooldown;
    float boostTimer;
    float cooldownTimer;
    
    // Animation state
    float mouthOpenAmount;
    float swimCycleTime;
    float targetPitch;  // Target pitch for smooth tilting
    
    // Boundaries
    float minX, maxX, minY, maxY, minZ, maxZ;
    
    std::unique_ptr<Model> model;
    
    Player(const std::string& modelPath);
    
    void Update(float deltaTime);
    void Draw(Shader& shader);
    
    void MoveInDirection(const glm::vec3& direction, float deltaTime);
    void MoveForward(float deltaTime);
    void MoveBackward(float deltaTime);
    void MoveLeft(float deltaTime);
    void MoveRight(float deltaTime);
    void MoveUp(float deltaTime);
    void MoveDown(float deltaTime);
    void Grow(float amount);
    float GetCollisionRadius() const;
    
    void SetSprint(bool sprint);
    void ActivateBoost();
    bool CanBoost() const;
    void UpdateRotation(float xoffset, float yoffset);
    
    void SetBoundaries(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
    
    glm::mat4 GetModelMatrix();

private:
    void updateVectors();
    void clampToBoundaries();
};

#endif
