#include "Player.h"
#include <algorithm>
#include <cmath>

Player::Player(const std::string& modelPath) 
    : position(0.0f, 0.0f, 0.0f),
      front(0.0f, 0.0f, -1.0f),
      up(0.0f, 1.0f, 0.0f),
      yaw(180.0f),  // Start facing forward (away from camera)
      pitch(0.0f),
      scale(0.1f),  // Scale down the model - most 3D models are quite large
      speed(10.0f),
      sprintMultiplier(2.0f),
      isSprinting(false),
      mouthOpenAmount(0.0f),
      swimCycleTime(0.0f),
      targetPitch(0.0f),
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
    
    // Smoothly interpolate pitch towards target
    float pitchSpeed = 5.0f;
    pitch += (targetPitch - pitch) * pitchSpeed * deltaTime;
    
    // Reset target pitch (will be set by movement functions if moving vertically)
    targetPitch = 0.0f;
    
    clampToBoundaries();
}

void Player::Draw(Shader& shader) {
    glm::mat4 modelMatrix = GetModelMatrix();
    shader.setMat4("model", modelMatrix);
    model->Draw(shader);
}

void Player::MoveInDirection(const glm::vec3& direction, float deltaTime) {
    float velocity = speed * deltaTime;
    if (isSprinting) velocity *= sprintMultiplier;
    
    // Move in the given direction
    position += direction * velocity;
    
    // Smoothly rotate fish to face movement direction
    // Calculate target yaw from direction (negate because of our rotation setup)
    float targetYaw = -atan2(direction.x, direction.z) * 180.0f / 3.14159f;
    
    // Smooth interpolation towards target yaw (like pitch system)
    float yawDiff = targetYaw - yaw;
    // Normalize angle difference to [-180, 180]
    while (yawDiff > 180.0f) yawDiff -= 360.0f;
    while (yawDiff < -180.0f) yawDiff += 360.0f;
    
    // Apply smooth interpolation (adjust 3.0f higher for faster, lower for slower)
    yaw += yawDiff * 5.0f * deltaTime;
    
    updateVectors();
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
    targetPitch = 30.0f;  // Tilt nose up when ascending
}

void Player::MoveDown(float deltaTime) {
    float velocity = speed * deltaTime;
    position -= up * velocity;
    targetPitch = -30.0f;  // Tilt nose down when descending
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
    
    // Apply base orientation fix (model's nose points up, rotate it down)
    // Base rotation to align model with world coordinates
    // Many 3D models are exported with Y-up or Z-up differently
    // If nose is pointing down, we need to adjust this base rotation
    // Try 0.0f (no rotation) if 90 was down and -90 was up
    // model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    
    // After base rotation, axes are remapped:
    // - Yaw (turning left/right) - Inverted Y to fix left/right direction
    model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, -1.0f, 0.0f));
    
    // - Pitch (nose up/down) - Inverted X to fix up/down direction
    model = glm::rotate(model, glm::radians(pitch), glm::vec3(-1.0f, 0.0f, 0.0f));
    
    // Add swimming animation (subtle body wave)
    float swimWave = sin(swimCycleTime * 4.0f) * 0.05f;
    model = glm::rotate(model, swimWave, glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Scale
    model = glm::scale(model, glm::vec3(scale));
    
    // Additional rotation to fix model orientation if needed
    // If the model faces +Z or -Z by default, we might need to rotate it 180 degrees
    // model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
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
