#ifndef GAME_H
#define GAME_H

#ifdef _WIN32
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
#else
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
#endif

#include <memory>
#include "Player.h"
#include "Camera.h"
#include "Ocean.h"
#include "Shader.h"
#include "Collectible.h"
#include "Enemy.h"
#include "TextRenderer.h"
#include <vector>

class Game {
public:
    Game(int width, int height);
    ~Game();
    
    bool Initialize();
    void Run();
    
    // Callbacks
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    int screenWidth;
    int screenHeight;
    GLFWwindow* window;
    
    std::unique_ptr<Player> player;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Ocean> ocean;
    std::unique_ptr<Shader> shader;
    std::vector<std::unique_ptr<Collectible>> collectibles;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::unique_ptr<Model> sunModel;
    std::unique_ptr<Shader> textShader;
    std::unique_ptr<TextRenderer> textRenderer;

    int score;
    bool gameOver;
    bool gameWon;
    float targetScale;
    
    // Timing
    float deltaTime;
    
    // Resources
    std::unique_ptr<Model> sharkModel;
    std::unique_ptr<Model> hookModel;
    std::unique_ptr<Model> fishModel;
    std::unique_ptr<Model> shellModel;
    float lastFrame;
    
    // Mouse
    float lastX;
    float lastY;
    bool firstMouse;
    
    // Input state
    bool keys[1024];
    bool leftMousePressed;
    
    void ProcessInput();
    void Update();
    void Render();
    
    void SetupOpenGL();
};

#endif
