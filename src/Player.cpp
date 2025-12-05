#include "Player.h"
#include <algorithm>
#include <cmath>

Player::Player(const std::string& modelPath) 
    : position(0.0f, 0.0f, 0.0f),
      front(0.0f, 0.0f, -1.0f),
      up(0.0f, 1.0f, 0.0f),
      yaw(-90.0f),
      pitch(0.0f),
      scale(0.1f),  // Scale down the model - most 3D models are quite large
      speed(10.0f),
      sprintMultiplier(2.0f),
      isSprinting(false),
      mouthOpenAmount(0.0f),
      swimCycleTime(0.0f),
      minX(-100.0f), maxX(100.0f),
      minY(-50.0f), maxY(50.0f),
      minZ(-100.0f), maxZ(100.0f) {
    
    model = std::make_unique<Model>(modelPath);
    updateVectors();
}

void Player::Update(float deltaTime) {
    // Update swim animation cycle
    swimCycleTime += deltaTime * 2.0f;
    
    // Mouth animation (simulates eating/breathing)
    mouthOpenAmount = (sin(swimCycleTime * 3.0f) + 1.0f) * 0.5f * 0.1f;
    
    clampToBoundaries();
}

void Player::Draw(Shader& shader) {
    glm::mat4 modelMatrix = GetModelMatrix();
    shader.setMat4("model", modelMatrix);
    model->Draw(shader);
}

void Player::MoveForward(float deltaTime) {
    float velocity = speed * deltaTime;
    if (isSprinting) velocity *= sprintMultiplier;
    position += front * velocity;
}

void Player::MoveBackward(float deltaTime) {
    float velocity = speed * deltaTime;
    if (isSprinting) velocity *= sprintMultiplier;
    position -= front * velocity;
}

void Player::MoveLeft(float deltaTime) {
    float velocity = speed * deltaTime;
    position -= right * velocity;
}

void Player::MoveRight(float deltaTime) {
    float velocity = speed * deltaTime;
    position += right * velocity;
}

void Player::MoveUp(float deltaTime) {
    float velocity = speed * deltaTime;
    position += up * velocity;
}

void Player::MoveDown(float deltaTime) {
    float velocity = speed * deltaTime;
    position -= up * velocity;
}

void Player::SetSprint(bool sprint) {
    isSprinting = sprint;
}

void Player::UpdateRotation(float xoffset, float yoffset) {
    xoffset *= 0.1f;
    yoffset *= 0.1f;
    
    yaw += xoffset;
    pitch += yoffset;
    
    // Constrain pitch
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    
    updateVectors();
}

void Player::SetBoundaries(float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
    this->minX = minX;
    this->maxX = maxX;
    this->minY = minY;
    this->maxY = maxY;
    this->minZ = minZ;
    this->maxZ = maxZ;
}

glm::mat4 Player::GetModelMatrix() {
    glm::mat4 model = glm::mat4(1.0f);
    
    // Translate to position
    model = glm::translate(model, position);
    
    // Rotate to face direction
    model = glm::rotate(model, glm::radians(yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(-pitch), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Add swimming animation (subtle body wave)
    float swimWave = sin(swimCycleTime * 4.0f) * 0.05f;
    model = glm::rotate(model, swimWave, glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Scale
    model = glm::scale(model, glm::vec3(scale));
    
    return model;
}

void Player::updateVectors() {
    // Calculate the new front vector
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    
    // Recalculate right and up vector
    right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    up = glm::normalize(glm::cross(right, front));
}

void Player::clampToBoundaries() {
    position.x = std::max(minX, std::min(maxX, position.x));
    position.y = std::max(minY, std::min(maxY, position.y));
    position.z = std::max(minZ, std::min(maxZ, position.z));
}
