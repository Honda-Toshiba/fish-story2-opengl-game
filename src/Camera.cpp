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

void Camera::FollowPlayer(const glm::vec3& playerPos, const glm::vec3& playerFront, float deltaTime) {
    if (mode == FIRST_PERSON) {
        // First person - camera at player position looking in player's direction
        Position = playerPos + glm::vec3(0.0f, 0.5f, 0.0f); // Slightly above fish center
        Front = playerFront;
    } else {
        // Third person - smooth camera that stays behind and above the player
        float distance = 15.0f;  // Distance behind the player
        float height = 6.0f;     // Height above the player
        
        // Calculate desired camera position (behind and above player)
        glm::vec3 horizontalFront = glm::normalize(glm::vec3(playerFront.x, 0.0f, playerFront.z));
        glm::vec3 desiredPosition = playerPos - horizontalFront * distance + glm::vec3(0.0f, height, 0.0f);
        
        // Smoothly interpolate camera position (smooth following)
        float smoothSpeed = 8.0f * deltaTime;
        Position = glm::mix(Position, desiredPosition, smoothSpeed);
        
        // Always look at the player position
        Front = glm::normalize(playerPos - Position);
    }
    
    // Update camera vectors
    Right = glm::normalize(glm::cross(Front, glm::vec3(0.0f, 1.0f, 0.0f)));
    Up = glm::normalize(glm::cross(Right, Front));
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
