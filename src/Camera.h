#ifndef CAMERA_H
#define CAMERA_H

#ifdef _WIN32
    #include <GL/glew.h>
#else
    #include <GL/glew.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

enum CameraMode {
    FIRST_PERSON,
    THIRD_PERSON
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 15.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
public:
    // Camera attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    
    // Euler angles
    float Yaw;
    float Pitch;
    
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    
    CameraMode mode;
    
    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
           float yaw = YAW, float pitch = PITCH);
    
    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix();
    
    // Processes input received from any keyboard-like input system
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    
    // Processes input received from a mouse input system
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    
    // Processes input received from a mouse scroll-wheel event
    void ProcessMouseScroll(float yoffset);
    
    // Update camera to follow player with bounds checking
    void FollowPlayer(const glm::vec3& playerPos, const glm::vec3& playerFront, float deltaTime, float playerScale,
                      float minX = -1000.0f, float maxX = 1000.0f, 
                      float minY = -1000.0f, float maxY = 1000.0f, 
                      float minZ = -1000.0f, float maxZ = 1000.0f);
    
    // Constrain camera position within bounds
    void ConstrainToBounds(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
    
    // Toggle camera mode
    void ToggleMode();

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();
};

#endif
