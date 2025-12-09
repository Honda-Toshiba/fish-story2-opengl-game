#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), 
      MouseSensitivity(SENSITIVITY), Zoom(ZOOM), mode(THIRD_PERSON) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
    if (direction == UP)
        Position += Up * velocity;
    if (direction == DOWN)
        Position -= Up * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // Update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

void Camera::FollowPlayer(const glm::vec3& playerPos, const glm::vec3& playerFront, float deltaTime, float playerScale) {
    if (mode == FIRST_PERSON) {
        // First person - adjust eye height based on fish size
        // Base height 0.5f scaled up
        float eyeHeight = 0.5f + (playerScale - 0.1f) * 2.0f;
        // Position camera slightly forward (2.0 units) so it's at the fish's eyes/front
        float forwardOffset = 2.0f * (playerScale / 0.1f);
        Position = playerPos + glm::vec3(0.0f, eyeHeight, 0.0f) + playerFront * forwardOffset;
        Front = playerFront;
        Up = glm::vec3(0.0f, 1.0f, 0.0f);
    } else {
        // Third person - orbit camera
        
        // DYNAMIC CAMERA DISTANCE
        // Base distance is 20.0f at scale 0.1f
        // As scale increases, we push the camera back linearly
        float scaleFactor = playerScale / 0.1f; // 1.0 at start, increases as you grow
        
        // Calculate new distance and height
        // We use a formula: Base + (Growth * Multiplier)
        float distance = 20.0f + (playerScale - 0.1f) * 40.0f; 
        float height = 3.0f + (playerScale - 0.1f) * 10.0f;
        
        // Camera position based on its own orientation (not player's)
        glm::vec3 offset = -Front * distance;
        offset.y += height;
        
        Position = playerPos + offset;
        
        // Update Right and Up vectors
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
}

void Camera::ToggleMode() {
    if (mode == FIRST_PERSON) {
        mode = THIRD_PERSON;
    } else {
        mode = FIRST_PERSON;
    }
}

void Camera::updateCameraVectors() {
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    
    // Also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
