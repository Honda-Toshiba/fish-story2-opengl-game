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

void Camera::FollowPlayer(const glm::vec3& playerPos, const glm::vec3& playerFront, float deltaTime, float playerScale,
                          float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
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
        // Third person - orbit camera with collision detection
        
        // DYNAMIC CAMERA DISTANCE
        // Base distance is 20.0f at scale 0.1f
        // As scale increases, we push the camera back linearly
        float scaleFactor = playerScale / 0.1f; // 1.0 at start, increases as you grow
        
        // Calculate desired distance and height
        float desiredDistance = 20.0f + (playerScale - 0.1f) * 40.0f; 
        float height = 3.0f + (playerScale - 0.1f) * 10.0f;
        
        // Camera offset based on its own orientation (not player's)
        glm::vec3 direction = -Front;
        
        // Try to find the maximum safe distance by checking for wall collisions
        float safeDistance = desiredDistance;
        float minDistance = 2.0f; // Minimum zoom distance
        
        // Check if the desired camera position would be out of bounds
        for (float testDist = desiredDistance; testDist >= minDistance; testDist -= 0.5f) {
            glm::vec3 testOffset = direction * testDist;
            testOffset.y += height;
            glm::vec3 testPos = playerPos + testOffset;
            
            // Check if this position is within bounds
            if (testPos.x >= minX && testPos.x <= maxX &&
                testPos.y >= minY && testPos.y <= maxY &&
                testPos.z >= minZ && testPos.z <= maxZ) {
                safeDistance = testDist;
                break;
            }
            
            // If we've reached minimum distance, use it
            if (testDist <= minDistance) {
                safeDistance = minDistance;
                break;
            }
        }
        
        // Apply the safe distance
        glm::vec3 offset = direction * safeDistance;
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

void Camera::ConstrainToBounds(float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
    // Clamp camera position to stay within bounds
    Position.x = glm::clamp(Position.x, minX, maxX);
    Position.y = glm::clamp(Position.y, minY, maxY);
    Position.z = glm::clamp(Position.z, minZ, maxZ);
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
