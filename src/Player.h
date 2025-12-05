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
    bool isSprinting;
    
    // Animation state
    float mouthOpenAmount;
    float swimCycleTime;
    
    // Boundaries
    float minX, maxX, minY, maxY, minZ, maxZ;
    
    std::unique_ptr<Model> model;
    
    Player(const std::string& modelPath);
    
    void Update(float deltaTime);
    void Draw(Shader& shader);
    
    void MoveForward(float deltaTime);
    void MoveBackward(float deltaTime);
    void MoveLeft(float deltaTime);
    void MoveRight(float deltaTime);
    void MoveUp(float deltaTime);
    void MoveDown(float deltaTime);
    
    void SetSprint(bool sprint);
    void UpdateRotation(float xoffset, float yoffset);
    
    void SetBoundaries(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
    
    glm::mat4 GetModelMatrix();

private:
    void updateVectors();
    void clampToBoundaries();
};

#endif
