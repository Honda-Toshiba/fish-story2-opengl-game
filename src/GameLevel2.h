#ifndef GAMELEVEL2_H
#define GAMELEVEL2_H

#ifdef _WIN32
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
#else
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
#endif

#include <memory>
#include <vector>
#include "Player.h"
#include "Camera.h"
#include "Cave.h"
#include "Shader.h"
#include "Anglerfish.h"
#include "Crab.h"
#include "TreasureChest.h"
#include "TextRenderer.h"
#include "Model.h"

class GameLevel2 {
public:
    GameLevel2(int width, int height);
    ~GameLevel2();
    
    bool Initialize();
    void Run();
    
    // Callbacks
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
    int screenWidth;
    int screenHeight;
    GLFWwindow* window;
    
    std::unique_ptr<Player> player;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Cave> cave;
    std::unique_ptr<Shader> shader;
    std::vector<std::unique_ptr<Anglerfish>> anglerfish;
    std::vector<std::unique_ptr<Crab>> crabs;
    std::unique_ptr<TreasureChest> treasureChest;
    
    // Model resources
    std::unique_ptr<Model> anglerfishModel;
    std::unique_ptr<Model> crabModel;
    std::unique_ptr<Model> treasureChestModel;
    std::unique_ptr<Model> coinModel;
    
    // UI resources
    std::unique_ptr<TextRenderer> textRenderer;
    std::unique_ptr<Shader> textShader;
    
    int score;
    int anglerfishCollected;
    bool gameOver;
    bool gameWon;
    
    // Health system
    float playerHealth;
    float maxHealth;
    float damageCooldown;
    float damageCooldownTimer;
    
    // Timing
    float deltaTime;
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
    void SpawnAnglerfish();
    void SpawnCrabs();
};

#endif
