#include "GameLevel2.h"
#include <iostream>
#include <cmath>
#include <cstdlib>

// Static pointer for callbacks
static GameLevel2* gameLevel2Instance = nullptr;

GameLevel2::GameLevel2(int width, int height)
    : screenWidth(width), screenHeight(height), window(nullptr),
      deltaTime(0.0f), lastFrame(0.0f),
      lastX(width / 2.0f), lastY(height / 2.0f), firstMouse(true),
      leftMousePressed(false), score(0), anglerfishCollected(0),
      gameOver(false), gameWon(false),
      stalactiteSpawnTimer(0.0f), stalactiteSpawnInterval(5.0f) {
    
    gameLevel2Instance = this;
    
    for (int i = 0; i < 1024; i++) {
        keys[i] = false;
    }
}

GameLevel2::~GameLevel2() {
    if (window) {
        glfwDestroyWindow(window);
        // Don't call glfwTerminate here - let main() handle it
    }
}

bool GameLevel2::Initialize() {
    // Initialize Audio Engine
    audio = std::make_unique<AudioEngine>();
    if (audio->Initialize()) {
        audio->LoadSound("coin-spill", "audio/coin-spill.mp3", false);
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
    window = glfwCreateWindow(screenWidth, screenHeight, "Fish Story 2 - Level 2: Dark Cave", NULL, NULL);
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
    glfwSetScrollCallback(window, ScrollCallback);
    
    // Capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK && glewError != GLEW_ERROR_NO_GLX_DISPLAY) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewError) << std::endl;
        return false;
    }
    
    // Clear any OpenGL errors
    while(glGetError() != GL_NO_ERROR);
    
    SetupOpenGL();
    
    // Initialize audio
    audio = std::make_unique<AudioEngine>();
    if (audio->Initialize()) {
        audio->LoadSound("bubbles", "audio/bubbles.mp3", true);
        audio->LoadSound("crunch", "audio/crunch.mp3", false);
        audio->Play("bubbles");
    }
    
    // Initialize text renderer
    textShader = std::make_unique<Shader>("shaders/text.vert", "shaders/text.frag");
    textRenderer = std::make_unique<TextRenderer>(screenWidth, screenHeight, textShader.get());
    textRenderer->Load("models/font_atlas.png", 32);
    
    // Initialize shader
    shader = std::make_unique<Shader>("shaders/vertex.glsl", "shaders/fragment.glsl");
    
    // Create cave - 350 units long, 30 units wide (wider!), 15 units tall
    cave = std::make_unique<Cave>(350.0f, 30.0f, 15.0f);
    
    // Create player and set initial position at cave entrance
    player = std::make_unique<Player>("models/Kingfish/Mesh_Kingfish.obj");
    player->position = glm::vec3(0.0f, 7.0f, 10.0f); // Start further into cave
    player->scale = 0.1f; // Same as Level 1
    player->yaw = 0.0f; // Face forward into cave
    
    // Set player boundaries based on cave size
    // Increased ceiling margin to prevent camera clipping through ceiling
    player->SetBoundaries(-cave->GetWidth()/2 + 2.0f, cave->GetWidth()/2 - 2.0f,
                          2.0f, cave->GetHeight() - 4.0f,
                          0.0f, cave->GetLength() - 5.0f);
    
    // Load Anglerfish model (now using the actual model!)
    std::string anglerfishPath = "models/Anglerfish/Anglerfish.glb";
    anglerfishModel = std::make_unique<Model>(anglerfishPath);
    
    // Load Crab model
    std::string crabPath = "models/Crab/Mesh_Crab.obj";
    crabModel = std::make_unique<Model>(crabPath);
    
    // Load Treasure Chest model
    std::string treasurePath = "models/Treasure chest/model-fine.obj";
    treasureChestModel = std::make_unique<Model>(treasurePath);
    
    // Load Coin model
    std::string coinPath = "models/Coin Dollar Sign/CoinDollarSign.obj";
    coinModel = std::make_unique<Model>(coinPath);
    std::cout << "Loaded coin model from: " << coinPath << std::endl;
    
    // Load Stalactite model
    std::string stalactitePath = "models/Rock (1)/PUSHILIN_rock.obj";
    stalactiteModel = std::make_unique<Model>(stalactitePath);
    std::cout << "Loaded stalactite model from: " << stalactitePath << std::endl;
    
    // Spawn anglerfish throughout the cave
    SpawnAnglerfish();
    
    // Spawn crabs on the floor
    SpawnCrabs();
    
    // Create treasure chest at the end of the cave
    glm::vec3 treasurePos(0.0f, 3.0f, cave->GetLength() - 10.0f); // Lowered to sit on floor properly
    treasureChest = std::make_unique<TreasureChest>(treasureChestModel.get(), coinModel.get(), treasurePos);
    
    // Create camera positioned close behind player at start
    camera = std::make_unique<Camera>(glm::vec3(0.0f, 8.0f, 7.0f));  // Right behind player
    camera->Yaw = 0.0f; // Face forward (down +Z axis)
    camera->Pitch = -10.0f; // Slight downward angle
    camera->mode = THIRD_PERSON; // Start in third person
    
    std::cout << "==================================================" << std::endl;
    std::cout << "    Level 2: Dark Underwater Cave" << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "Mission: Navigate through the dark cave!" << std::endl;
    std::cout << "Collect Anglerfish to light your way!" << std::endl;
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move forward/backward/left/right" << std::endl;
    std::cout << "  Space/Shift - Move up/down" << std::endl;
    std::cout << "  Hold Left Mouse - Sprint" << std::endl;
    std::cout << "  Keys 1/2 - Switch camera mode" << std::endl;
    std::cout << "  Mouse - Look around" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "==================================================" << std::endl;
    
    return true;
}

void GameLevel2::SetupOpenGL() {
    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Set viewport
    glViewport(0, 0, screenWidth, screenHeight);
}

void GameLevel2::SpawnAnglerfish() {
    // Spawn fewer Anglerfish throughout the cave (was 15, now 8 for better spacing)
    // Distribute them evenly along the length of the cave
    float caveLength = cave->GetLength();
    float spacing = caveLength / 9.0f; // 9 sections for 8 fish
    
    for (int i = 1; i <= 8; i++) {
        float z = spacing * i;
        
        // Vary X position (left, center, right)
        float x = 0.0f;
        if (i % 3 == 0) x = -6.0f;      // Left
        else if (i % 3 == 1) x = 6.0f;  // Right
        // else x = 0.0f (center)
        
        // Vary Y position (low, mid, high)
        float y = 5.0f + (i % 3) * 3.0f; // Range: 5, 8, 11
        
        glm::vec3 pos(x, y, z);
        anglerfish.push_back(std::make_unique<Anglerfish>(anglerfishModel.get(), pos, 50.0f)); // Much larger scale - model is tiny! (was 2.0f)
    }
    
    std::cout << "Spawned " << anglerfish.size() << " Anglerfish in the cave!" << std::endl;
    std::cout << "Cave dimensions: " << cave->GetLength() << "x" << cave->GetWidth() << "x" << cave->GetHeight() << std::endl;
    std::cout << "Player starting at: " << player->position.x << ", " << player->position.y << ", " << player->position.z << std::endl;
    
    // Start with one Anglerfish already following the player as a companion
    if (!anglerfish.empty()) {
        anglerfish[0]->Collect();
        anglerfishCollected = 1;
        score = 20; // Give starting score for the companion
        std::cout << "Starting companion Anglerfish lighting your way!" << std::endl;
    }
}

void GameLevel2::SpawnCrabs() {
    // Spawn crabs on the floor along the cave
    float caveLength = cave->GetLength();
    float caveWidth = cave->GetWidth();
    float spacing = caveLength / 6.0f; // 6 sections for 5 crabs
    
    // Cave is 30 units wide, so walls are at ±15
    // We want crabs to patrol from wall to wall with small margins
    float leftWall = -caveWidth/2.0f + 2.0f;   // -13
    float rightWall = caveWidth/2.0f - 2.0f;   // +13
    
    for (int i = 1; i <= 5; i++) {
        float z = spacing * i;
        
        // Start at left or right wall
        float startX = (i % 2 == 0) ? leftWall : rightWall;
        float y = 0.5f; // On the floor
        
        // Calculate patrol range: distance from start to opposite wall
        float patrolRange = (i % 2 == 0) ? (rightWall - leftWall) : (rightWall - leftWall);
        
        glm::vec3 pos(startX, y, z);
        crabs.push_back(std::make_unique<Crab>(crabModel.get(), pos, patrolRange));
    }
    
    std::cout << "Spawned " << crabs.size() << " crabs in the cave (wall-to-wall patrol)!" << std::endl;
}

void GameLevel2::Run() {
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

void GameLevel2::ProcessInput() {
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (gameOver || gameWon) {
            // Reset logic
            gameOver = false;
            gameWon = false;
            score = 0;
            anglerfishCollected = 0;
            player->position = glm::vec3(0.0f, 7.0f, 5.0f);
            
            // Respawn anglerfish
            anglerfish.clear();
            SpawnAnglerfish();
            
            std::cout << "Level 2 Restarted!" << std::endl;
        }
    }
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // Player movement
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
    
    if (isMoving) {
        moveDirection = glm::normalize(glm::vec3(moveDirection.x, 0.0f, moveDirection.z));
        // In first-person, don't rotate player to face movement (mouse controls rotation)
        // In third-person, rotate player to face movement direction
        bool shouldRotate = (camera->mode == THIRD_PERSON);
        player->MoveInDirection(moveDirection, deltaTime, shouldRotate);
    }
    
    // Vertical movement
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        player->MoveUp(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        player->MoveDown(deltaTime);
    
    // Burst Sprint with left mouse button
    if (leftMousePressed && !player->isSprinting && player->sprintCooldownTimer <= 0.0f) {
        player->isSprinting = true;
        player->sprintTimer = 0.0f;
        player->speed = 25.0f; // Apply Burst Speed immediately
    }
}

void GameLevel2::Update() {
    if (gameOver) return; // Only stop on game over, not on win
    
    player->Update(deltaTime);
    
    // Update sprint system
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
    
    cave->Update(deltaTime);
    
    // Update camera to follow player with cave bounds for collision detection
    // Add margin to prevent camera from clipping through walls
    // Increased ceiling margin to 4.0f to prevent clipping when player is high
    float margin = 2.0f;
    float ceilingMargin = 4.0f;
    camera->FollowPlayer(
        player->position, 
        player->front, 
        deltaTime, 
        player->scale,
        -cave->GetWidth()/2.0f + margin,  // minX
        cave->GetWidth()/2.0f - margin,    // maxX
        margin,                             // minY (above floor)
        cave->GetHeight() - ceilingMargin,  // maxY (below ceiling)
        margin,                             // minZ (past front wall)
        cave->GetLength() - margin          // maxZ (before back wall)
    );
    
    // Debug: print player position every 60 frames
    static int frameCount = 0;
    if (frameCount++ % 60 == 0) {
        std::cout << "Player pos: (" << player->position.x << ", " << player->position.y << ", " << player->position.z << ")" << std::endl;
        std::cout << "Camera pos: (" << camera->Position.x << ", " << camera->Position.y << ", " << camera->Position.z << ")" << std::endl;
    }
    
    // Update and check anglerfish
    for (auto& fish : anglerfish) {
        fish->Update(deltaTime, player->position);
        
        if (fish->CheckCollision(player->position, 2.0f)) {
            fish->Collect();
            anglerfishCollected++;
            score += 20;
        }
    }
    
    // Update and check crabs
    for (auto& crab : crabs) {
        crab->Update(deltaTime, player->position);
        
        if (crab->CheckCollision(player->position, 2.0f)) {
            std::cout << "*** GAME OVER! *** Hit by crab!" << std::endl;
            gameOver = true;
            return; // Stop updating immediately
        }
    }
    
    // Update treasure chest
    if (treasureChest) {
        treasureChest->Update(deltaTime);
        
        // Check if player reaches treasure (only check collision if not already won)
        if (!gameWon && !treasureChest->IsOpened() && treasureChest->CheckCollision(player->position, 2.0f)) {
            treasureChest->Open();
            std::cout << "*** TREASURE FOUND! *** Coins flying!" << std::endl;
            audio->Play("coin-spill");
            gameWon = true;
            score += 100;
        }
    }
    
    // Update stalactites
    for (auto it = stalactites.begin(); it != stalactites.end();) {
        (*it)->Update(deltaTime);
        
        // Check collision with player
        if ((*it)->CheckCollision(player->position, 2.0f)) {
            std::cout << "*** GAME OVER! *** Hit by stalactite!" << std::endl;
            gameOver = true;
            return;
        }
        
        // Remove inactive stalactites
        if (!(*it)->IsActive()) {
            it = stalactites.erase(it);
        } else {
            ++it;
        }
    }
    
    // Spawn new stalactites periodically
    stalactiteSpawnTimer += deltaTime;
    if (stalactiteSpawnTimer >= stalactiteSpawnInterval) {
        SpawnStalactite();
        stalactiteSpawnTimer = 0.0f;
    }
    
    // Check win condition - reached the end of the cave (backup)
    if (player->position.z >= cave->GetLength() - 5.0f && !gameWon) {
        std::cout << "*** YOU WIN! *** You made it through the cave!" << std::endl;
        gameWon = true;
    }
    
    // Update window title
    std::string title = "Fish Story 2 - Level 2 | Score: " + std::to_string(score) + 
                        " | Anglerfish: " + std::to_string(anglerfishCollected) + "/8" +
                        " | Progress: " + std::to_string((int)(cave->GetProgress(player->position) * 100)) + "%";
    glfwSetWindowTitle(window, title.c_str());
}

void GameLevel2::Render() {
    // Cave ambient - much darker for atmosphere
    glm::vec3 caveAmbient(0.08f, 0.08f, 0.1f); // Even darker (was 0.15)
    
    if (gameOver) {
        caveAmbient = glm::vec3(0.5f, 0.0f, 0.0f); // Red for game over
    } else if (gameWon) {
        caveAmbient = glm::vec3(0.2f, 0.5f, 0.2f); // Green for victory
    }
    
    glClearColor(caveAmbient.r, caveAmbient.g, caveAmbient.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    shader->use();
    
    // Setup view and projection matrices
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom),
                                           (float)screenWidth / (float)screenHeight,
                                           0.1f, 500.0f);
    
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setVec3("viewPos", camera->Position);
    
    // Setup multiple light sources from collected anglerfish
    int lightCount = 0;
    shader->setInt("numLights", 0); // Will be updated
    
    // Add collected anglerfish as light sources
    for (auto& fish : anglerfish) {
        if (fish->ProvidesLight() && lightCount < 8) { // Max 8 lights
            std::string lightBase = "lights[" + std::to_string(lightCount) + "]";
            shader->setVec3(lightBase + ".position", fish->GetLightPosition()); // Use light position, not fish center
            shader->setVec3(lightBase + ".color", fish->GetLightColor());
            shader->setFloat(lightBase + ".intensity", fish->GetLightIntensity());
            shader->setFloat(lightBase + ".radius", fish->GetLightRadius()); // Use actual radius from fish
            lightCount++;
        }
    }
    
    // Debug: Print light count once at startup
    static bool debugPrinted = false;
    if (!debugPrinted) {
        std::cout << "Active lights: " << lightCount << " (should be 1 at start)" << std::endl;
        debugPrinted = true;
    }
    
    shader->setInt("numLights", lightCount);
    
    // Set ambient lighting for cave - increased for better visibility
    glm::vec3 ambientLight = glm::vec3(0.12f, 0.12f, 0.15f); // Increased from 0.05 for better base visibility
    shader->setVec3("ambientLight", ambientLight);
    
    // Legacy uniforms for compatibility
    shader->setVec3("lightPos", player->position + glm::vec3(0.0f, 2.0f, 0.0f));
    shader->setVec3("lightColor", 0.4f, 0.4f, 0.5f);
    shader->setVec3("skyColor", caveAmbient);
    shader->setFloat("time", glfwGetTime());
    
    // Initialize all shader flags
    shader->setBool("isSun", false);
    shader->setBool("isWater", false);
    shader->setBool("isSkybox", false);
    shader->setBool("isFloor", false);
    shader->setBool("isGlowing", false);
    shader->setBool("hasTexture", false);
    shader->setFloat("glowIntensity", 0.0f);
    
    // Draw cave
    cave->Draw(*shader);
    
    // Draw player first
    shader->setVec3("objectColor", 0.3f, 0.5f, 0.6f);
    shader->setBool("isGlowing", false);
    player->Draw(*shader);
    
    // Draw anglerfish with glow
    for (auto& fish : anglerfish) {
        fish->Draw(*shader);
    }
    
    // Draw crabs
    for (auto& crab : crabs) {
        crab->Draw(*shader);
    }
    
    // Draw stalactites
    for (auto& stalactite : stalactites) {
        stalactite->Draw(*shader);
    }
    
    // Draw treasure chest and coins
    if (treasureChest) {
        treasureChest->Draw(*shader);
    }
    
    // ---------------------------------------------
    // DRAW TEXT / HUD (Always)
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
        std::string msg1 = "LEVEL 2 COMPLETE!";
        std::string msg2 = "Press 'R' to Continue";
        float centerX = screenWidth / 2.0f;
        float centerY = screenHeight / 2.0f;
        
        // Draw Gold Text on Green Background
        textRenderer->RenderText(msg1, centerX - 190.0f, centerY + 20.0f, 2.0f, glm::vec3(1.0f, 0.9f, 0.0f));
        textRenderer->RenderText(msg2, centerX - 190.0f, centerY - 50.0f, 1.2f, glm::vec3(1.0f, 1.0f, 1.0f));
    }
    else {
        // 1. Standard Stats (Top Left)
        std::string scoreText = "Score: " + std::to_string(score);
        textRenderer->RenderText(scoreText, 20.0f, screenHeight - 50.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        
        std::string fishText = "Anglerfish: " + std::to_string(anglerfishCollected) + "/8";
        textRenderer->RenderText(fishText, 20.0f, screenHeight - 90.0f, 1.0f, glm::vec3(0.5f, 0.8f, 1.0f));
        
        std::string progressText = "Progress: " + std::to_string((int)(cave->GetProgress(player->position) * 100)) + "%";
        textRenderer->RenderText(progressText, 20.0f, screenHeight - 130.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.5f));
        
        // 2. SPRINT / COOLDOWN BAR (Bottom Center)
        glDisable(GL_DEPTH_TEST);
        float barWidth = 300.0f;
        float barHeight = 15.0f;
        
        float barX = (screenWidth / 2.0f) - (barWidth / 2.0f);
        float barY = 40.0f;
        
        // Draw Background (Dark Gray)
        textRenderer->RenderBar(barX, barY, barWidth, barHeight, glm::vec3(0.2f, 0.2f, 0.2f));

        // Calculate Fill & Color
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
        
        // Draw Foreground Bar
        textRenderer->RenderBar(barX, barY, barWidth * fillPercent, barHeight, barColor);
        
        // Draw Text (Centered above the bar)
        float textX = (screenWidth / 2.0f) - (statusText.length() * 4.0f);
        textRenderer->RenderText(statusText, textX, barY + 25.0f, 0.5f, barColor);
        
        glEnable(GL_DEPTH_TEST);
    }
    glDisable(GL_BLEND);
}

// Callback implementations
void GameLevel2::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (gameLevel2Instance) {
        gameLevel2Instance->screenWidth = width;
        gameLevel2Instance->screenHeight = height;
    }
}

void GameLevel2::MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!gameLevel2Instance) return;
    
    if (gameLevel2Instance->firstMouse) {
        gameLevel2Instance->lastX = xpos;
        gameLevel2Instance->lastY = ypos;
        gameLevel2Instance->firstMouse = false;
    }
    
    float xoffset = xpos - gameLevel2Instance->lastX;
    float yoffset = gameLevel2Instance->lastY - ypos;
    
    gameLevel2Instance->lastX = xpos;
    gameLevel2Instance->lastY = ypos;
    
    // In first-person mode, mouse controls player rotation
    // In third-person mode, mouse controls camera orbit
    if (gameLevel2Instance->camera->mode == FIRST_PERSON) {
        gameLevel2Instance->player->UpdateRotation(xoffset, yoffset);
    } else {
        gameLevel2Instance->camera->ProcessMouseMovement(xoffset, yoffset);
    }
}

void GameLevel2::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (!gameLevel2Instance) return;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS)
            gameLevel2Instance->leftMousePressed = true;
        else if (action == GLFW_RELEASE)
            gameLevel2Instance->leftMousePressed = false;
    }
}

void GameLevel2::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (!gameLevel2Instance) return;
    
    if (action == GLFW_PRESS) {
        // Camera mode switching
        if (key == GLFW_KEY_1 || key == GLFW_KEY_2) {
            gameLevel2Instance->camera->ToggleMode();
            std::cout << "Camera mode: " << 
                (gameLevel2Instance->camera->mode == FIRST_PERSON ? "First Person" : "Third Person") << std::endl;
        }
    }
}

void GameLevel2::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (!gameLevel2Instance) return;
    gameLevel2Instance->camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

void GameLevel2::SpawnStalactite() {
    // Spawn 2-4 stalactites at random positions throughout the cave
    int numStalactites = 2 + (rand() % 3); // 2, 3, or 4 stalactites
    
    float caveWidth = cave->GetWidth();
    float caveHeight = cave->GetHeight();
    float caveLength = cave->GetLength();
    
    for (int i = 0; i < numStalactites; i++) {
        // Random X position within cave width
        float randomX = ((float)rand() / RAND_MAX - 0.5f) * (caveWidth - 4.0f);
        
        // Spawn anywhere from current player position to end of cave
        // This creates stalactites both near and far from the player
        float minZ = player->position.z - 10.0f; // Some behind
        float maxZ = caveLength - 20.0f; // Not too close to end
        
        // Clamp minZ to valid range
        if (minZ < 10.0f) minZ = 10.0f;
        
        float spawnZ = minZ + ((float)rand() / RAND_MAX) * (maxZ - minZ);
        
        // Start at ceiling
        float spawnY = caveHeight - 1.0f;
        
        glm::vec3 spawnPos(randomX, spawnY, spawnZ);
        
        // Random hang time between 1.0 and 4.0 seconds
        float hangTime = 1.0f + ((float)rand() / RAND_MAX) * 3.0f;
        
        stalactites.push_back(std::make_unique<Stalactite>(stalactiteModel.get(), spawnPos, hangTime));
        
        std::cout << "Stalactite " << (i+1) << "/" << numStalactites << " spawned at (" 
                  << randomX << ", " << spawnY << ", " << spawnZ << ") - hang time: " << hangTime << "s" << std::endl;
    }
}
