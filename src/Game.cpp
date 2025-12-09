#include "Game.h"
#include <iostream>
#include <sstream>
#include <iomanip>

// Static pointer for callbacks
static Game* gameInstance = nullptr;

Game::Game(int width, int height)
    : screenWidth(width), screenHeight(height), window(nullptr),
      deltaTime(0.0f), lastFrame(0.0f),
      lastX(width / 2.0f), lastY(height / 2.0f), firstMouse(true),
      leftMousePressed(false), score(0.0f), gameOver(false), gameWon(false), targetScale(0.2f),
      speedBoostActive(false), speedBoostTimer(0.0f), speedBoostDuration(10.0f),
      doubleScoreActive(false), doubleScoreTimer(0.0f), doubleScoreDuration(15.0f) {
    
    gameInstance = this;
    
    for (int i = 0; i < 1024; i++) {
        keys[i] = false;
    }
}

Game::~Game() {
    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

bool Game::Initialize() {
    audio = std::make_unique<AudioEngine>();
    if (audio->Initialize()) {
        // Make sure you have these files in an 'audio' folder!
        audio->LoadSound("bubbles", "audio/bubbles.mp3", true); // Loop = true
        audio->LoadSound("crunch", "audio/crunch.mp3", false);  // One-shot
        audio->LoadSound("damage", "audio/damage.mp3", false);
        audio->LoadSound("hook", "audio/hook.mp3", false);
        
        // Start ambient noise immediately [cite: 36]
        audio->Play("bubbles");
    }
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // Set OpenGL version (3.3)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    window = glfwCreateWindow(screenWidth, screenHeight, "Fish Story 2 - Level 1", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetKeyCallback(window, KeyCallback);
    
    // Capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK && glewError != GLEW_ERROR_NO_GLX_DISPLAY) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewError) << std::endl;
        std::cerr << "GLEW error code: " << glewError << std::endl;
        return false;
    }
    
    // Clear any OpenGL errors that may have occurred during GLEW initialization
    while(glGetError() != GL_NO_ERROR);
    
    SetupOpenGL();
    
    textShader = std::make_unique<Shader>("shaders/text.vert", "shaders/text.frag");
    textRenderer = std::make_unique<TextRenderer>(screenWidth, screenHeight, textShader.get());

    textRenderer->Load("models/font_atlas.png", 32);
    // Initialize game objects
    shader = std::make_unique<Shader>("shaders/vertex.glsl", "shaders/fragment.glsl");
    ocean = std::make_unique<Ocean>(100.0f, 50.0f);
    
    // Create player and set initial position
    player = std::make_unique<Player>("models/Kingfish/Mesh_Kingfish.obj");
    player->position = glm::vec3(0.0f, -20.0f, 0.0f); // Start in middle of ocean
    
    // Load models once
    std::string shellPath = "models/Seashell/seaShell2.obj"; 
    shellModel = std::make_unique<Model>(shellPath);
    
    std::string smallFishPath = "models/Fish_v1_L2.123ce045555c-e177-486e-8ce8-dad39381ed15/12265_Fish_v1_L2.obj";
    fishModel = std::make_unique<Model>(smallFishPath);
    
    std::string sharkPath = "models/Shark/shark.obj";
    sharkModel = std::make_unique<Model>(sharkPath);
    
    std::string hookPath = "models/Hook/Fish Hook.obj";
    hookModel = std::make_unique<Model>(hookPath);

    std::string sunPath = "models/Sun/model.obj"; 
    sunModel = std::make_unique<Model>(sunPath);

    // Create collectibles (Seashells with different powerups)
    // Green shells - Speed boost
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(10.0f, -25.0f, 10.0f), 0.5f, SPEED_BOOST));
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(-15.0f, -20.0f, 5.0f), 0.5f, SPEED_BOOST));
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(5.0f, -30.0f, -15.0f), 0.5f, SPEED_BOOST));
    // Yellow shells - Double score
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(-20.0f, -15.0f, -20.0f), 0.5f, DOUBLE_SCORE));
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(25.0f, -22.0f, 0.0f), 0.5f, DOUBLE_SCORE));
    
    // Add many swimming fish scattered around the map using Fish_v1 model
    float fishScale = 0.15f; // Small fish
    
    // Distribute 30 swimming fish around the map at various positions
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(15.0f, -18.0f, 20.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-20.0f, -25.0f, 15.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(30.0f, -12.0f, -25.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-35.0f, -20.0f, -10.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(8.0f, -30.0f, 12.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-12.0f, -15.0f, 30.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(25.0f, -28.0f, -18.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-28.0f, -22.0f, 22.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(18.0f, -16.0f, -8.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-8.0f, -35.0f, -20.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(40.0f, -20.0f, 8.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-40.0f, -18.0f, -15.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(5.0f, -24.0f, 35.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-15.0f, -28.0f, -30.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(32.0f, -14.0f, 18.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-25.0f, -32.0f, 5.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(10.0f, -19.0f, -28.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-18.0f, -26.0f, 28.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(28.0f, -22.0f, -12.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-32.0f, -16.0f, 18.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(12.0f, -33.0f, -5.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-5.0f, -20.0f, -35.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(35.0f, -25.0f, 25.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-22.0f, -30.0f, -8.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(22.0f, -17.0f, 15.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-10.0f, -23.0f, 32.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(38.0f, -29.0f, -22.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-38.0f, -19.0f, 12.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(2.0f, -27.0f, -18.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-16.0f, -21.0f, 8.0f), FISH, fishScale));

    // Create Enemies (Sharks and Hooks)
    // Add 4 patrolling sharks positioned to cover the entire 2D plane
    // Each shark patrols in a circle with radius 20.0f
    // Position them strategically to cover all quadrants
    enemies.push_back(std::make_unique<Enemy>(sharkModel.get(), glm::vec3(35.0f, -15.0f, 35.0f), SHARK, 1.5f));   // Top-right quadrant
    enemies.push_back(std::make_unique<Enemy>(sharkModel.get(), glm::vec3(-35.0f, -15.0f, 35.0f), SHARK, 1.5f));  // Top-left quadrant
    enemies.push_back(std::make_unique<Enemy>(sharkModel.get(), glm::vec3(35.0f, -15.0f, -35.0f), SHARK, 1.5f));  // Bottom-right quadrant
    enemies.push_back(std::make_unique<Enemy>(sharkModel.get(), glm::vec3(-35.0f, -15.0f, -35.0f), SHARK, 1.5f)); // Bottom-left quadrant
    
    // Add several small dangling hooks distributed across the entire water area
    // Made very small (0.03f scale) and spread out across different depths and positions
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(20.0f, -12.0f, 25.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(-25.0f, -18.0f, -20.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(40.0f, -15.0f, -15.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(-30.0f, -20.0f, 30.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(0.0f, -10.0f, -35.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(15.0f, -22.0f, 0.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(-40.0f, -14.0f, 10.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(30.0f, -25.0f, -30.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(-15.0f, -16.0f, 40.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(5.0f, -28.0f, 15.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(-5.0f, -11.0f, -25.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(35.0f, -19.0f, 20.0f), HOOK, 0.03f));

    // Create camera at proper initial position behind the player
    camera = std::make_unique<Camera>(glm::vec3(0.0f, -10.0f, 25.0f));
    
    // Set player boundaries based on ocean size
    player->SetBoundaries(-ocean->GetSize(), ocean->GetSize(),
                          -ocean->GetDepth() + 5.0f, 8.0f,
                          -ocean->GetSize(), ocean->GetSize());
    
    std::cout << "Game initialized successfully!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move forward/backward/left/right" << std::endl;
    std::cout << "  Space/Shift - Move up/down" << std::endl;
    std::cout << "  Hold Left Mouse - Sprint" << std::endl;
    std::cout << "  E - Boost Sprint (2s duration, 5s cooldown)" << std::endl;
    std::cout << "  Keys 1/2 - Switch camera mode" << std::endl;
    std::cout << "  Mouse - Look around" << std::endl;
    std::cout << "  R - Restart (when game over)" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "\nObjective: Eat 4 glowing fish to reach score 2.0 and win!" << std::endl;
    std::cout << "Avoid sharks and hooks or you'll die!" << std::endl;
    
    return true;
}

void Game::SetupOpenGL() {
    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Set viewport
    glViewport(0, 0, screenWidth, screenHeight);
}

void Game::Run() {
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        ProcessInput();
        Update();
        Render();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Game::ProcessInput() {
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (gameOver || gameWon) {
            ResetLevel();
        }
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // Player movement - track if any movement key is pressed
    bool isMoving = false;
    glm::vec3 moveDirection(0.0f);
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        moveDirection += camera->Front;
        isMoving = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        moveDirection -= camera->Front;
        isMoving = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        moveDirection -= camera->Right;
        isMoving = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        moveDirection += camera->Right;
        isMoving = true;
    }
    
    // Apply horizontal movement relative to camera
    if (isMoving) {
        // Normalize to prevent faster diagonal movement
        moveDirection = glm::normalize(glm::vec3(moveDirection.x, 0.0f, moveDirection.z));
        // In first-person, don't rotate player to face movement (mouse controls rotation)
        // In third-person, rotate player to face movement direction
        bool shouldRotate = (camera->mode == THIRD_PERSON);
        player->MoveInDirection(moveDirection, deltaTime, shouldRotate);
    }
    
    // Vertical movement (not affected by camera)
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        player->MoveUp(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        player->MoveDown(deltaTime);
    
    if (leftMousePressed && !player->isSprinting && player->sprintCooldownTimer <= 0.0f) {
        player->isSprinting = true;
        player->sprintTimer = 0.0f;
        player->speed = 25.0f; // Apply Burst Speed immediately
        // Optional: Play sound
        // audio->Play("dash"); 
    }
            
    // T key for next level (restarts for now)
    if (gameWon && glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        gameWon = false;
        gameOver = false;
        score = 0;
        player->position = glm::vec3(0.0f, -20.0f, 0.0f);
    }
}

void Game::ResetLevel() {
    // 1. Reset Player & Game State
    gameOver = false;
    gameWon = false;
    score = 0.0f;
    
    player->scale = 0.1f;
    player->position = glm::vec3(0.0f, -20.0f, 0.0f);
    player->speed = 10.0f; // Reset speed (in case boost was active)
    
    // Reset Powerups
    speedBoostActive = false;
    doubleScoreActive = false;
    speedBoostTimer = 0.0f;
    doubleScoreTimer = 0.0f;

    // 2. Clear Old Entities
    enemies.clear();
    collectibles.clear();

    // 3. RESPATCH COLLECTIBLES (Shells)
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(10.0f, -25.0f, 10.0f), 0.5f, SPEED_BOOST));
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(-15.0f, -20.0f, 5.0f), 0.5f, SPEED_BOOST));
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(5.0f, -30.0f, -15.0f), 0.5f, SPEED_BOOST));
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(-20.0f, -15.0f, -20.0f), 0.5f, DOUBLE_SCORE));
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(25.0f, -22.0f, 0.0f), 0.5f, DOUBLE_SCORE));

    // 4. RESPATCH ENEMIES (Fish, Sharks, Hooks)
    float fishScale = 0.15f; 
    
    // Edible Fish
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(15.0f, -18.0f, 20.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-20.0f, -25.0f, 15.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(30.0f, -12.0f, -25.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-35.0f, -20.0f, -10.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(8.0f, -30.0f, 12.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-12.0f, -15.0f, 30.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(25.0f, -28.0f, -18.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-28.0f, -22.0f, 22.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(18.0f, -16.0f, -8.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-8.0f, -35.0f, -20.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(40.0f, -20.0f, 8.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-40.0f, -18.0f, -15.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(5.0f, -24.0f, 35.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-15.0f, -28.0f, -30.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(32.0f, -14.0f, 18.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-25.0f, -32.0f, 5.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(10.0f, -19.0f, -28.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-18.0f, -26.0f, 28.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(28.0f, -22.0f, -12.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-32.0f, -16.0f, 18.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(12.0f, -33.0f, -5.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-5.0f, -20.0f, -35.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(35.0f, -25.0f, 25.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-22.0f, -30.0f, -8.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(22.0f, -17.0f, 15.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-10.0f, -23.0f, 32.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(38.0f, -29.0f, -22.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-38.0f, -19.0f, 12.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(2.0f, -27.0f, -18.0f), FISH, fishScale));
    enemies.push_back(std::make_unique<Enemy>(fishModel.get(), glm::vec3(-16.0f, -21.0f, 8.0f), FISH, fishScale));

    // Sharks
    enemies.push_back(std::make_unique<Enemy>(sharkModel.get(), glm::vec3(35.0f, -15.0f, 35.0f), SHARK, 1.5f));
    enemies.push_back(std::make_unique<Enemy>(sharkModel.get(), glm::vec3(-35.0f, -15.0f, 35.0f), SHARK, 1.5f));
    enemies.push_back(std::make_unique<Enemy>(sharkModel.get(), glm::vec3(35.0f, -15.0f, -35.0f), SHARK, 1.5f));
    enemies.push_back(std::make_unique<Enemy>(sharkModel.get(), glm::vec3(-35.0f, -15.0f, -35.0f), SHARK, 1.5f));

    // Hooks
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(20.0f, -12.0f, 25.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(-25.0f, -18.0f, -20.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(40.0f, -15.0f, -15.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(-30.0f, -20.0f, 30.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(0.0f, -10.0f, -35.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(15.0f, -22.0f, 0.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(-40.0f, -14.0f, 10.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(30.0f, -25.0f, -30.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(-15.0f, -16.0f, 40.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(5.0f, -28.0f, 15.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(-5.0f, -11.0f, -25.0f), HOOK, 0.03f));
    enemies.push_back(std::make_unique<Enemy>(hookModel.get(), glm::vec3(35.0f, -19.0f, 20.0f), HOOK, 0.03f));

    std::cout << "Game Level Reset!" << std::endl;
    audio->Play("bubbles");
}

void Game::Update() {
    // 1. Stop updating if game is over
    if (gameOver || gameWon) return;

    // 2. Standard Updates
    player->Update(deltaTime);
    ocean->Update(deltaTime);
    camera->FollowPlayer(player->position, player->front, deltaTime, player->scale);
    
    // 3. Update Collectibles & Powerups
    // Update powerup timers
    if (speedBoostActive) {
        speedBoostTimer += deltaTime;
        if (speedBoostTimer >= speedBoostDuration) {
            speedBoostActive = false;
            speedBoostTimer = 0.0f;
            player->speed = 10.0f; // Reset to normal speed
        }
    }
    if (doubleScoreActive) {
        doubleScoreTimer += deltaTime;
        if (doubleScoreTimer >= doubleScoreDuration) {
            doubleScoreActive = false;
            doubleScoreTimer = 0.0f;
        }
    }
    
    for (auto& collectible : collectibles) {
        collectible->Update(deltaTime);
        if (collectible->IsActive()) {
            if (collectible->CheckCollision(player->position, player->GetCollisionRadius())) {
                collectible->Deactivate();
                
                // Activate powerup based on type
                if (collectible->GetPowerUpType() == SPEED_BOOST) {
                    speedBoostActive = true;
                    speedBoostTimer = 0.0f;
                    player->speed = 20.0f; // Double speed
                } else if (collectible->GetPowerUpType() == DOUBLE_SCORE) {
                    doubleScoreActive = true;
                    doubleScoreTimer = 0.0f;
                }
            }
        }
    }
    
    if (player->isSprinting) {
        // We are currently running fast
        player->sprintTimer += deltaTime;
        if (player->sprintTimer >= player->sprintDuration) {
            // Time's up! Stop sprinting.
            player->isSprinting = false;
            player->speed = 10.0f; // Reset to normal speed
            player->sprintCooldownTimer = player->sprintCooldown; // Start the cooldown
            player->sprintTimer = 0.0f;
        }
    } 
    else {
        // We are not sprinting, check cooldown
        if (player->sprintCooldownTimer > 0.0f) {
            player->sprintCooldownTimer -= deltaTime;
        }
    }

    // 4. Update Enemies & Check Collisions
    float playerRadius = player->GetCollisionRadius();
    
    float minFishDistance = 1000.0f;

    for (auto it = enemies.begin(); it != enemies.end(); ) {
        (*it)->Update(deltaTime, player->position);
        
        // CASE A: Edible Fish - touch to eat
        if ((*it)->GetType() == FISH) {
            // Calculate distance for Audio Logic
            float dist = glm::distance((*it)->GetPosition(), player->position);
            if (dist < minFishDistance) {
                minFishDistance = dist;
            }

            // Check collision for Eating Logic
            if ((*it)->CheckCollision(player->position, playerRadius)) {
                if (!(*it)->IsBeingEaten()) {
                    // Calculate mouth position (in front of player)
                    glm::vec3 mouthPos = player->position + player->front * 12.5f;
                    
                    float scoreGain = 0.5f;
                    player->scale += 0.005f;
                    if (doubleScoreActive) scoreGain *= 2.0f; 
                    score += scoreGain;
                    
                    // Play crunch sound on eat
                    audio->Play("crunch");
                    
                    // Trigger player eating animation
                    player->TriggerEatingAnimation();
                    
                    // Start eating animation instead of immediate removal
                    (*it)->StartEatingAnimation(mouthPos);
                }
            }
            
            // Remove fish after eating animation completes
            if ((*it)->IsEatenComplete()) {
                it = enemies.erase(it);
                continue;
            }
        }
        // CASE B: Dangerous Enemies (Shark/Hook) - touch to die
        else {
            if ((*it)->CheckCollision(player->position, playerRadius)) {
                if (!gameOver) {
                    // --- NEW: Play specific sound based on enemy type ---
                    if ((*it)->GetType() == SHARK) {
                        audio->Play("crunch"); // Shark bite
                        audio->Play("damage"); // Generic hit
                    } 
                    else if ((*it)->GetType() == HOOK) {
                        audio->Play("hook");   // Metal hook sound
                        // You can play "damage" here too if you want, 
                        // or just let the hook sound stand alone.
                    }
                    audio->Stop("bubbles"); // Stop ambience for dramatic effect
                }
                
                std::cout << "!!! GAME OVER !!! You were caught by a " 
                         << ((*it)->GetType() == SHARK ? "shark" : "hook") << "!" << std::endl;
                std::cout << "Final Score: " << score << std::endl;
                gameOver = true;
            }
        }
        
        ++it;
    }

    // --- NEW: ADJUST BUBBLE VOLUME BASED ON PROXIMITY ---
    if (!gameOver && !gameWon) {
        float maxHearingDist = 25.0f; // Distance where sound fades to 0
        float volume = 0.0f;

        if (minFishDistance < maxHearingDist) {
            // Linear fade: 1.0 (Close) -> 0.0 (Far)
            volume = 1.0f - (minFishDistance / maxHearingDist);
        }

        // Clamp volume to be safe
        volume = std::max(0.0f, std::min(volume, 1.0f));
        
        // Optional: Keep a tiny bit of background noise always (e.g. 0.05)
        // volume = std::max(0.05f, volume);

        audio->SetVolume("bubbles", volume);
    }

    // 5. Check WIN Condition (Score-based)
    if (player->scale >= targetScale) {
        std::cout << "*** YOU WIN! *** You collected enough fish!" << std::endl;
        std::cout << "Final Score: " << score << std::endl;
        gameWon = true;
    }

    // 6. Update HUD (Window Title)
    std::string title = "Fish Story 2";
    glfwSetWindowTitle(window, title.c_str());
}

void Game::Render() {
    // ---------------------------------------------
    // 1. CELESTIAL MATH (The Physics)
    // ---------------------------------------------
    float cycleTime = glfwGetTime() * 0.1f; 
    float orbitRadius = 100.0f;
    
    float orbitX = sin(cycleTime) * orbitRadius;
    float orbitY = cos(cycleTime) * orbitRadius;
    
    glm::vec3 sunPos(orbitX, orbitY, 0.0f);
    glm::vec3 moonPos = -sunPos; 

    glm::vec3 activeLightPos;
    float skyMix;

    if (sunPos.y >= 0.0f) {
        activeLightPos = sunPos;
        skyMix = (sin(cycleTime) + 1.0f) / 2.0f; 
    } else {
        activeLightPos = moonPos;
        skyMix = 0.0f; 
    }

    // ---------------------------------------------
    // 2. BACKGROUND COLOR SETUP
    // ---------------------------------------------
    glm::vec3 dayLight(1.0f, 0.95f, 0.8f);    
    glm::vec3 nightLight(0.05f, 0.05f, 0.2f); 
    glm::vec3 daySky(0.1f, 0.3f, 0.5f);       
    glm::vec3 nightSky(0.01f, 0.01f, 0.05f);  

    glm::vec3 currentLightColor = glm::mix(nightLight, dayLight, skyMix);
    glm::vec3 currentSkyColor   = glm::mix(nightSky, daySky, skyMix);

    // FIX: Set Clear Color based on state
    if (gameOver) {
        glClearColor(0.5f, 0.0f, 0.0f, 1.0f); // Red for Loss
    } 
    else if (gameWon) {
        glClearColor(0.0f, 0.35f, 0.0f, 1.0f); // Green for Win
    } 
    else {
        glClearColor(currentSkyColor.r, currentSkyColor.g, currentSkyColor.b, 1.0f); // Normal Sky
    }

    // Clear Screen ONCE
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ---------------------------------------------
    // 3. DRAW 3D WORLD (Only if Playing)
    // ---------------------------------------------
    // FIX: We skip this huge block if the game is over/won so the screen stays clean
    if (!gameOver && !gameWon) {
        shader->use();

    // Send Uniforms
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom),
                                           (float)screenWidth / (float)screenHeight,
                                           0.1f, 500.0f);
    
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    
    // Send "Real" Light Source Position
    shader->setVec3("lightPos", activeLightPos);
    shader->setVec3("viewPos", camera->Position);
    shader->setVec3("lightColor", currentLightColor);
    shader->setVec3("skyColor", currentSkyColor);
    shader->setFloat("time", glfwGetTime());

        // Draw Sun
        if (sunPos.y > -20.0f) {
            glm::mat4 sunMat = glm::mat4(1.0f);
            sunMat = glm::translate(sunMat, sunPos);
            sunMat = glm::scale(sunMat, glm::vec3(8.0f)); 
            shader->setMat4("model", sunMat);
            shader->setVec3("objectColor", 1.00f, 0.80f, 0.00f); 
            shader->setBool("isSun", true);
            if (sunModel) sunModel->Draw(*shader);
            else shellModel->Draw(*shader);
            shader->setBool("isSun", false); 
        }

        // Draw Moon
        if (moonPos.y > -20.0f) {
            glm::mat4 moonMat = glm::mat4(1.0f);
            moonMat = glm::translate(moonMat, moonPos);
            moonMat = glm::scale(moonMat, glm::vec3(4.0f)); 
            shader->setMat4("model", moonMat);
            shader->setVec3("objectColor", 0.9f, 0.9f, 1.0f); 
            shader->setBool("isSun", true); 
            if (sunModel) sunModel->Draw(*shader);
            else shellModel->Draw(*shader);
            shader->setBool("isSun", false); 
        }

        // Draw World
        ocean->Draw(*shader); 
        for (auto& collectible : collectibles) collectible->Draw(*shader);
        
        shader->setVec3("objectColor", 0.5f, 0.5f, 0.5f);
        shader->setBool("isGlowingFish", false);
        for (auto& enemy : enemies) {
            if (enemy->GetType() == FISH) shader->setBool("isGlowingFish", true);
            enemy->Draw(*shader);
            shader->setBool("isGlowingFish", false);
        }
        
        // Only draw player model in third-person mode
        if (camera->mode == THIRD_PERSON) {
            player->Draw(*shader);
        }
    }

    // ---------------------------------------------
    // 4. DRAW TEXT / HUD (Always)
    // ---------------------------------------------
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    if (gameOver) {
        std::string msg1 = "GAME OVER";
        std::string msg2 = "Press 'R' to Restart";
        float centerX = screenWidth / 2.0f;
        float centerY = screenHeight / 2.0f;
        
        // Draw White Text on Red Background
        textRenderer->RenderText(msg1, centerX - 180.0f, centerY + 20.0f, 2.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        textRenderer->RenderText(msg2, centerX - 190.0f, centerY - 50.0f, 1.2f, glm::vec3(1.0f, 1.0f, 1.0f));
    }
    else if (gameWon) {
        std::string msg1 = "LEVEL 1 COMPLETE!";
        std::string msg2 = "Press 'R' to Continue";
        float centerX = screenWidth / 2.0f;
        float centerY = screenHeight / 2.0f;
        
        // Draw Gold Text on Green Background
        textRenderer->RenderText(msg1, centerX - 190.0f, centerY + 20.0f, 2.0f, glm::vec3(1.0f, 0.9f, 0.0f));
        textRenderer->RenderText(msg2, centerX - 190.0f, centerY - 50.0f, 1.2f, glm::vec3(1.0f, 1.0f, 1.0f));
    }
    else {
        // 1. Standard Stats (Top Left)
        std::string scoreText = "Score: " + std::to_string((int)score);
        textRenderer->RenderText(scoreText, 20.0f, screenHeight - 50.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << (player->scale * 100) << " / " << targetScale * 100;
        std::string sizeText = "Size: " + ss.str();
        textRenderer->RenderText(sizeText, 20.0f, screenHeight - 90.0f, 1.0f, glm::vec3(0.5f, 0.8f, 1.0f)); 

        // 2. POWERUP INDICATORS (Top Right)
        float buffX = screenWidth - 300.0f; // Start 300px from right edge
        float buffY = screenHeight - 50.0f; // Start at top
        glDisable(GL_DEPTH_TEST);
        // -- SPEED BOOST --
        // 2. SPRINT / COOLDOWN BAR (Bottom Center)
        float barWidth = 300.0f; // Make it wider for the main HUD
        float barHeight = 15.0f;
        
        // Math to center items horizontally: (Screen/2) - (Item/2)
        float barX = (screenWidth / 2.0f) - (barWidth / 2.0f);
        float barY = 40.0f; // 40 pixels from the bottom edge
        
        // A. Draw Background (Dark Gray)
        textRenderer->RenderBar(barX, barY, barWidth, barHeight, glm::vec3(0.2f, 0.2f, 0.2f));

        // B. Calculate Fill & Color
        float fillPercent = 1.0f;
        glm::vec3 barColor = glm::vec3(1.0f); 
        std::string statusText = "";

        if (player->isSprinting) {
            // Draining (Green)
            fillPercent = 1.0f - (player->sprintTimer / player->sprintDuration);
            barColor = glm::vec3(0.0f, 1.0f, 0.0f); 
            statusText = "SPRINTING";
        } 
        else if (player->sprintCooldownTimer > 0.0f) {
            // Recharging (Orange)
            fillPercent = 1.0f - (player->sprintCooldownTimer / player->sprintCooldown);
            barColor = glm::vec3(1.0f, 0.5f, 0.0f); 
            statusText = "RECHARGING";
        } 
        else {
            // Ready (Cyan)
            fillPercent = 1.0f;
            barColor = glm::vec3(0.0f, 1.0f, 1.0f); 
            statusText = "READY [LMB]";
        }
        
        glEnable(GL_DEPTH_TEST);
        // C. Draw Foreground Bar
        textRenderer->RenderBar(barX, barY, barWidth * fillPercent, barHeight, barColor);
        
        // D. Draw Text (Centered above the bar)
        // A rough estimate to center text: Subtract ~4px per character from center
        float textX = (screenWidth / 2.0f) - (statusText.length() * 4.0f);
        textRenderer->RenderText(statusText, textX, barY + 25.0f, 0.5f, barColor);
        
        // Draw Foreground Bar
        // We scale the Width based on percentage
        textRenderer->RenderBar(barX, barY, barWidth * fillPercent, barHeight, barColor);

        // -- DOUBLE SCORE --
        if (doubleScoreActive) {
            float timeLeft = doubleScoreDuration - doubleScoreTimer;
            
            std::stringstream ds;
            ds << "2X SCORE: " << std::fixed << std::setprecision(1) << timeLeft << "s";
            
            // Render in Gold
            textRenderer->RenderText(ds.str(), buffX, buffY, 1.0f, glm::vec3(1.0f, 0.8f, 0.0f));
        }
    }
    glDisable(GL_BLEND);
}

void Game::RenderEndScreen(const std::string& message, float r, float g, float b) {
    // Clear to colored background
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Completely disable shader program - use fixed pipeline
    glUseProgram(0);
    
    // Disable depth test for 2D overlay
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    // Set up orthographic projection for 2D rendering
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);  // Y flipped: 0 at top
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Draw huge white letters
    glColor3f(1.0f, 1.0f, 1.0f);
    
    float startY = screenHeight * 0.3f;
    float letterSize = 60.0f;
    float spacing = 70.0f;
    
    if (message.find("WIN") != std::string::npos) {
        float startX = screenWidth * 0.25f;
        
        // Y
        glBegin(GL_QUADS);
        glVertex2f(startX, startY); glVertex2f(startX + 30, startY);
        glVertex2f(startX + 30, startY + 30); glVertex2f(startX, startY + 30);
        glVertex2f(startX + 30, startY + 30); glVertex2f(startX + 60, startY + 30);
        glVertex2f(startX + 60, startY + 60); glVertex2f(startX + 30, startY + 60);
        glVertex2f(startX + 60, startY); glVertex2f(startX + 90, startY);
        glVertex2f(startX + 90, startY + 30); glVertex2f(startX + 60, startY + 30);
        glEnd();
        startX += spacing;
        
        // O
        glBegin(GL_QUADS);
        glVertex2f(startX + 10, startY); glVertex2f(startX + 80, startY);
        glVertex2f(startX + 80, startY + 20); glVertex2f(startX + 10, startY + 20);
        glVertex2f(startX, startY + 20); glVertex2f(startX + 20, startY + 20);
        glVertex2f(startX + 20, startY + 40); glVertex2f(startX, startY + 40);
        glVertex2f(startX + 70, startY + 20); glVertex2f(startX + 90, startY + 20);
        glVertex2f(startX + 90, startY + 40); glVertex2f(startX + 70, startY + 40);
        glVertex2f(startX + 10, startY + 40); glVertex2f(startX + 80, startY + 40);
        glVertex2f(startX + 80, startY + 60); glVertex2f(startX + 10, startY + 60);
        glEnd();
        startX += spacing;
        
        // U
        glBegin(GL_QUADS);
        glVertex2f(startX, startY); glVertex2f(startX + 20, startY);
        glVertex2f(startX + 20, startY + 40); glVertex2f(startX, startY + 40);
        glVertex2f(startX + 70, startY); glVertex2f(startX + 90, startY);
        glVertex2f(startX + 90, startY + 40); glVertex2f(startX + 70, startY + 40);
        glVertex2f(startX + 10, startY + 40); glVertex2f(startX + 80, startY + 40);
        glVertex2f(startX + 80, startY + 60); glVertex2f(startX + 10, startY + 60);
        glEnd();
        startX += spacing + 30;
        
        // W
        glBegin(GL_QUADS);
        glVertex2f(startX, startY); glVertex2f(startX + 20, startY);
        glVertex2f(startX + 20, startY + 60); glVertex2f(startX, startY + 60);
        glVertex2f(startX + 35, startY + 40); glVertex2f(startX + 55, startY + 40);
        glVertex2f(startX + 55, startY + 60); glVertex2f(startX + 35, startY + 60);
        glVertex2f(startX + 70, startY); glVertex2f(startX + 90, startY);
        glVertex2f(startX + 90, startY + 60); glVertex2f(startX + 70, startY + 60);
        glEnd();
        startX += spacing + 20;
        
        // I
        glBegin(GL_QUADS);
        glVertex2f(startX + 20, startY); glVertex2f(startX + 50, startY);
        glVertex2f(startX + 50, startY + 60); glVertex2f(startX + 20, startY + 60);
        glEnd();
        startX += spacing - 20;
        
        // N
        glBegin(GL_QUADS);
        glVertex2f(startX, startY); glVertex2f(startX + 20, startY);
        glVertex2f(startX + 20, startY + 60); glVertex2f(startX, startY + 60);
        glVertex2f(startX + 20, startY + 20); glVertex2f(startX + 70, startY + 20);
        glVertex2f(startX + 70, startY + 40); glVertex2f(startX + 20, startY + 40);
        glVertex2f(startX + 70, startY); glVertex2f(startX + 90, startY);
        glVertex2f(startX + 90, startY + 60); glVertex2f(startX + 70, startY + 60);
        glEnd();
        
    } else {
        float startX = screenWidth * 0.15f;
        float lineY = startY;
        
        // G
        glBegin(GL_QUADS);
        glVertex2f(startX + 10, lineY); glVertex2f(startX + 90, lineY);
        glVertex2f(startX + 90, lineY + 20); glVertex2f(startX + 10, lineY + 20);
        glVertex2f(startX, lineY + 20); glVertex2f(startX + 20, lineY + 20);
        glVertex2f(startX + 20, lineY + 60); glVertex2f(startX, lineY + 60);
        glVertex2f(startX + 10, lineY + 60); glVertex2f(startX + 90, lineY + 60);
        glVertex2f(startX + 90, lineY + 80); glVertex2f(startX + 10, lineY + 80);
        glVertex2f(startX + 50, lineY + 35); glVertex2f(startX + 90, lineY + 35);
        glVertex2f(startX + 90, lineY + 55); glVertex2f(startX + 50, lineY + 55);
        glEnd();
        startX += spacing + 20;
        
        // A
        glBegin(GL_QUADS);
        glVertex2f(startX + 30, lineY); glVertex2f(startX + 60, lineY);
        glVertex2f(startX + 60, lineY + 20); glVertex2f(startX + 30, lineY + 20);
        glVertex2f(startX, lineY + 20); glVertex2f(startX + 20, lineY + 20);
        glVertex2f(startX + 20, lineY + 80); glVertex2f(startX, lineY + 80);
        glVertex2f(startX + 70, lineY + 20); glVertex2f(startX + 90, lineY + 20);
        glVertex2f(startX + 90, lineY + 80); glVertex2f(startX + 70, lineY + 80);
        glVertex2f(startX + 20, lineY + 40); glVertex2f(startX + 70, lineY + 40);
        glVertex2f(startX + 70, lineY + 55); glVertex2f(startX + 20, lineY + 55);
        glEnd();
        startX += spacing + 20;
        
        // M
        glBegin(GL_QUADS);
        glVertex2f(startX, lineY); glVertex2f(startX + 20, lineY);
        glVertex2f(startX + 20, lineY + 80); glVertex2f(startX, lineY + 80);
        glVertex2f(startX + 30, lineY + 20); glVertex2f(startX + 50, lineY + 20);
        glVertex2f(startX + 50, lineY + 50); glVertex2f(startX + 30, lineY + 50);
        glVertex2f(startX + 80, lineY); glVertex2f(startX + 100, lineY);
        glVertex2f(startX + 100, lineY + 80); glVertex2f(startX + 80, lineY + 80);
        glEnd();
        startX += spacing + 40;
        
        // E
        glBegin(GL_QUADS);
        glVertex2f(startX, lineY); glVertex2f(startX + 80, lineY);
        glVertex2f(startX + 80, lineY + 20); glVertex2f(startX, lineY + 20);
        glVertex2f(startX, lineY); glVertex2f(startX + 20, lineY);
        glVertex2f(startX + 20, lineY + 80); glVertex2f(startX, lineY + 80);
        glVertex2f(startX, lineY + 30); glVertex2f(startX + 70, lineY + 30);
        glVertex2f(startX + 70, lineY + 50); glVertex2f(startX, lineY + 50);
        glVertex2f(startX, lineY + 60); glVertex2f(startX + 80, lineY + 60);
        glVertex2f(startX + 80, lineY + 80); glVertex2f(startX, lineY + 80);
        glEnd();
        startX += spacing + 30;
        
        // Second line: OVER
        lineY = startY + 120;
        startX = screenWidth * 0.2f;
        
        // O
        glBegin(GL_QUADS);
        glVertex2f(startX + 10, lineY); glVertex2f(startX + 80, lineY);
        glVertex2f(startX + 80, lineY + 20); glVertex2f(startX + 10, lineY + 20);
        glVertex2f(startX, lineY + 20); glVertex2f(startX + 20, lineY + 20);
        glVertex2f(startX + 20, lineY + 60); glVertex2f(startX, lineY + 60);
        glVertex2f(startX + 70, lineY + 20); glVertex2f(startX + 90, lineY + 20);
        glVertex2f(startX + 90, lineY + 60); glVertex2f(startX + 70, lineY + 60);
        glVertex2f(startX + 10, lineY + 60); glVertex2f(startX + 80, lineY + 60);
        glVertex2f(startX + 80, lineY + 80); glVertex2f(startX + 10, lineY + 80);
        glEnd();
        startX += spacing + 20;
        
        // V
        glBegin(GL_QUADS);
        glVertex2f(startX, lineY); glVertex2f(startX + 20, lineY);
        glVertex2f(startX + 20, lineY + 50); glVertex2f(startX, lineY + 50);
        glVertex2f(startX + 70, lineY); glVertex2f(startX + 90, lineY);
        glVertex2f(startX + 90, lineY + 50); glVertex2f(startX + 70, lineY + 50);
        glVertex2f(startX + 30, lineY + 50); glVertex2f(startX + 60, lineY + 50);
        glVertex2f(startX + 60, lineY + 80); glVertex2f(startX + 30, lineY + 80);
        glEnd();
        startX += spacing + 20;
        
        // E
        glBegin(GL_QUADS);
        glVertex2f(startX, lineY); glVertex2f(startX + 80, lineY);
        glVertex2f(startX + 80, lineY + 20); glVertex2f(startX, lineY + 20);
        glVertex2f(startX, lineY); glVertex2f(startX + 20, lineY);
        glVertex2f(startX + 20, lineY + 80); glVertex2f(startX, lineY + 80);
        glVertex2f(startX, lineY + 30); glVertex2f(startX + 70, lineY + 30);
        glVertex2f(startX + 70, lineY + 50); glVertex2f(startX, lineY + 50);
        glVertex2f(startX, lineY + 60); glVertex2f(startX + 80, lineY + 60);
        glVertex2f(startX + 80, lineY + 80); glVertex2f(startX, lineY + 80);
        glEnd();
        startX += spacing + 20;
        
        // R
        glBegin(GL_QUADS);
        glVertex2f(startX, lineY); glVertex2f(startX + 20, lineY);
        glVertex2f(startX + 20, lineY + 80); glVertex2f(startX, lineY + 80);
        glVertex2f(startX + 20, lineY); glVertex2f(startX + 80, lineY);
        glVertex2f(startX + 80, lineY + 20); glVertex2f(startX + 20, lineY + 20);
        glVertex2f(startX + 70, lineY + 20); glVertex2f(startX + 90, lineY + 20);
        glVertex2f(startX + 90, lineY + 35); glVertex2f(startX + 70, lineY + 35);
        glVertex2f(startX + 20, lineY + 35); glVertex2f(startX + 80, lineY + 35);
        glVertex2f(startX + 80, lineY + 50); glVertex2f(startX + 20, lineY + 50);
        glVertex2f(startX + 60, lineY + 50); glVertex2f(startX + 90, lineY + 50);
        glVertex2f(startX + 90, lineY + 80); glVertex2f(startX + 60, lineY + 80);
        glEnd();
    }
    
    // Only print to console once
    static std::string lastMessage = "";
    if (message != lastMessage) {
        std::cout << "\n========================================" << std::endl;
        std::cout << message << std::endl;
        std::cout << "========================================\n" << std::endl;
        lastMessage = message;
    }
    
    // Re-enable depth test
    glEnable(GL_DEPTH_TEST);
}

// Callback implementations
void Game::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (gameInstance) {
        gameInstance->screenWidth = width;
        gameInstance->screenHeight = height;
        
        // Update text renderer with new dimensions
        if (gameInstance->textRenderer) {
            gameInstance->textRenderer->UpdateScreenSize(width, height);
        }
        
        // Update camera projection matrix for new aspect ratio
        if (gameInstance->camera) {
            // Camera will use new aspect ratio on next render
        }
    }
}

void Game::MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!gameInstance) return;
    
    if (gameInstance->firstMouse) {
        gameInstance->lastX = xpos;
        gameInstance->lastY = ypos;
        gameInstance->firstMouse = false;
    }
    
    float xoffset = xpos - gameInstance->lastX;
    float yoffset = gameInstance->lastY - ypos; // Reversed since y-coordinates go from bottom to top
    
    gameInstance->lastX = xpos;
    gameInstance->lastY = ypos;
    
    // In first-person mode, mouse controls player rotation
    // In third-person mode, mouse controls camera orbit
    if (gameInstance->camera->mode == FIRST_PERSON) {
        gameInstance->player->UpdateRotation(xoffset, yoffset);
    } else {
        gameInstance->camera->ProcessMouseMovement(xoffset, yoffset);
    }
}

void Game::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (!gameInstance) return;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS)
            gameInstance->leftMousePressed = true;
        else if (action == GLFW_RELEASE)
            gameInstance->leftMousePressed = false;
    }
}

void Game::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (!gameInstance) return;
    
    if (action == GLFW_PRESS) {
        // Camera mode switching
        if (key == GLFW_KEY_1 || key == GLFW_KEY_2) {
            gameInstance->camera->ToggleMode();
            std::cout << "Camera mode: " << 
                (gameInstance->camera->mode == FIRST_PERSON ? "First Person" : "Third Person") << std::endl;
        }
    }
}
