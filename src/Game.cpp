#include "Game.h"
#include <iostream>

// Static pointer for callbacks
static Game* gameInstance = nullptr;

Game::Game(int width, int height)
    : screenWidth(width), screenHeight(height), window(nullptr),
      deltaTime(0.0f), lastFrame(0.0f),
      lastX(width / 2.0f), lastY(height / 2.0f), firstMouse(true),
      leftMousePressed(false), score(0), gameOver(false), gameWon(false), targetScale(2.0f) {
    
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

    // Create collectibles (Seashells)
    // Add some shells at random positions
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(10.0f, -25.0f, 10.0f), 0.5f));
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(-15.0f, -20.0f, 5.0f), 0.5f));
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(5.0f, -30.0f, -15.0f), 0.5f));
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(-20.0f, -15.0f, -20.0f), 0.5f));
    collectibles.push_back(std::make_unique<Collectible>(shellModel.get(), glm::vec3(25.0f, -22.0f, 0.0f), 0.5f));
    
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
    std::cout << "  Keys 1/2 - Switch camera mode" << std::endl;
    std::cout << "  Mouse - Look around" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    
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
            // Reset Logic
            gameOver = false;
            gameWon = false;
            score = 0;
            player->scale = 0.1f; // Reset size
            player->position = glm::vec3(0.0f, -20.0f, 0.0f);
            std::cout << "Game Restarted!" << std::endl;
            // Ideally, you would respawn eaten fish here too
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
        player->MoveInDirection(moveDirection, deltaTime);
    }
    
    // Vertical movement (not affected by camera)
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        player->MoveUp(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        player->MoveDown(deltaTime);
    
    // Sprint with left mouse button
    player->SetSprint(leftMousePressed);
}

void Game::Update() {
    // 1. Stop updating if game is over
    if (gameOver || gameWon) return;

    // 2. Standard Updates
    player->Update(deltaTime);
    ocean->Update(deltaTime);
    camera->FollowPlayer(player->position, player->front, deltaTime, player->scale);
    
    // 3. Update Collectibles
    for (auto& collectible : collectibles) {
        collectible->Update(deltaTime);
        if (collectible->IsActive()) {
            if (collectible->CheckCollision(player->position, 2.0f)) {
                collectible->Deactivate();
                score += 10;
                // Optional: Shells give a small growth boost?
                // player->Grow(0.05f); 
            }
        }
    }
    
    // 4. Update Enemies & Check Collisions
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        (*it)->Update(deltaTime, player->position);
        
        // CASE A: Edible Fish
        if ((*it)->GetType() == FISH) {
            // Check if we eat the fish (Dynamic collision radius based on size)
            float eatRadius = 1.5f * (player->scale / 0.1f);
            
            if (glm::distance((*it)->GetPosition(), player->position) < eatRadius) {
                score += 5;
                player->Grow(0.02f); // Grow per fish
                it = enemies.erase(it); // Remove eaten fish
                continue;
            }
        }
        // CASE B: Dangerous Enemies (Shark/Hook)
        else if ((*it)->CheckCollision(player->position, 1.0f)) {
            std::cout << "!!! GAME OVER !!! You were caught." << std::endl;
            gameOver = true;
        }
        
        ++it;
    }

    // 5. Check WIN Condition
    if (player->scale >= targetScale) {
        std::cout << "*** YOU WIN! *** King of the Ocean!" << std::endl;
        gameWon = true;
    }

    // 6. Update HUD (Window Title)
    std::string title = "Fish Story 2 | Score: " + std::to_string(score) + 
                        " | Size: " + std::to_string(player->scale) + 
                        " / " + std::to_string(targetScale);
    glfwSetWindowTitle(window, title.c_str());
}

void Game::Render() {
    // ---------------------------------------------
    // 1. CELESTIAL MATH (The Physics)
    // ---------------------------------------------
    // Cycle: 0.0 to 2*PI. Multiplier 0.1f = Day speed.
    float cycleTime = glfwGetTime() * 0.1f; 
    float orbitRadius = 100.0f;
    
    // Calculate positions (Opposite to each other)
    float orbitX = sin(cycleTime) * orbitRadius;
    float orbitY = cos(cycleTime) * orbitRadius;
    
    // Position vectors based on orbit
    glm::vec3 sunPos(orbitX, orbitY, 0.0f);
    glm::vec3 moonPos = -sunPos; 

    // ---------------------------------------------
    // 2. DETERMINE ACTIVE LIGHT SOURCE
    // ---------------------------------------------
    glm::vec3 activeLightPos;
    float skyMix;

    if (sunPos.y >= 0.0f) {
        // DAYTIME
        activeLightPos = sunPos;
        skyMix = (sin(cycleTime) + 1.0f) / 2.0f; // Smooth day/night transition
    } else {
        // NIGHTTIME
        activeLightPos = moonPos;
        skyMix = 0.0f; // Night
    }

    // ---------------------------------------------
    // 3. RENDER SETUP
    // ---------------------------------------------
    // Define Atmosphere Colors
    glm::vec3 dayLight(1.0f, 0.95f, 0.8f);    
    glm::vec3 nightLight(0.05f, 0.05f, 0.2f); 
    
    glm::vec3 daySky(0.1f, 0.3f, 0.5f);       
    glm::vec3 nightSky(0.01f, 0.01f, 0.05f);  

    glm::vec3 currentLightColor = glm::mix(nightLight, dayLight, skyMix);
    glm::vec3 currentSkyColor   = glm::mix(nightSky, daySky, skyMix);

    // Clear screen
    if (gameOver) {
        // Red tint for Death
        currentSkyColor = glm::vec3(0.5f, 0.0f, 0.0f);
        currentLightColor = glm::vec3(1.0f, 0.0f, 0.0f);
    } 
    else if (gameWon) {
        // Gold/Green tint for Victory
        currentSkyColor = glm::vec3(0.8f, 0.7f, 0.2f);
        currentLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    }

    // Clear buffers
    glClearColor(currentSkyColor.r, currentSkyColor.g, currentSkyColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
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

    // ==========================================
    // 4. DRAW CELESTIAL BODIES (FIRST!)
    // ==========================================
    // We draw these before the water so they appear "behind" the transparent surface.
    
    // -- DRAW SUN --
    if (sunPos.y > -20.0f) {
        glm::mat4 sunMat = glm::mat4(1.0f);
        sunMat = glm::translate(sunMat, sunPos);
        sunMat = glm::scale(sunMat, glm::vec3(8.0f)); 

        shader->setMat4("model", sunMat);
        shader->setVec3("objectColor", 1.00f, 0.80f, 0.00f); // Yellow
        shader->setBool("isSun", true); // Trigger Glow Shader
        
        if (sunModel) sunModel->Draw(*shader);
        else shellModel->Draw(*shader);
        
        shader->setBool("isSun", false); 
    }

    // -- DRAW MOON --
    if (moonPos.y > -20.0f) {
        glm::mat4 moonMat = glm::mat4(1.0f);
        moonMat = glm::translate(moonMat, moonPos);
        moonMat = glm::scale(moonMat, glm::vec3(4.0f)); 

        shader->setMat4("model", moonMat);
        shader->setVec3("objectColor", 0.9f, 0.9f, 1.0f); // White
        shader->setBool("isSun", true); // Reuse Glow Shader
        
        if (sunModel) sunModel->Draw(*shader);
        else shellModel->Draw(*shader);
        
        shader->setBool("isSun", false); 
    }

    // ==========================================
    // 5. DRAW WORLD (Water blends over Sun)
    // ==========================================
    ocean->Draw(*shader); 
    
    for (auto& collectible : collectibles) collectible->Draw(*shader);
    
    shader->setVec3("objectColor", 0.5f, 0.5f, 0.5f);
    for (auto& enemy : enemies) enemy->Draw(*shader);
    
    player->Draw(*shader);
}

// Callback implementations
void Game::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (gameInstance) {
        gameInstance->screenWidth = width;
        gameInstance->screenHeight = height;
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
    
    // Mouse controls camera, not player
    gameInstance->camera->ProcessMouseMovement(xoffset, yoffset);
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
