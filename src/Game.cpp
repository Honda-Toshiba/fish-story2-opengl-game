#include "Game.h"
#include <iostream>

// Static pointer for callbacks
static Game* gameInstance = nullptr;

Game::Game(int width, int height)
    : screenWidth(width), screenHeight(height), window(nullptr),
      deltaTime(0.0f), lastFrame(0.0f),
      lastX(width / 2.0f), lastY(height / 2.0f), firstMouse(true),
      leftMousePressed(false) {
    
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
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    
    SetupOpenGL();
    
    // Initialize game objects
    shader = std::make_unique<Shader>("shaders/vertex.glsl", "shaders/fragment.glsl");
    ocean = std::make_unique<Ocean>(100.0f, 50.0f);
    
    // Create player and set initial position
    player = std::make_unique<Player>("models/Kingfish/Mesh_Kingfish.obj");
    player->position = glm::vec3(0.0f, -20.0f, 0.0f); // Start in middle of ocean
    
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
    player->Update(deltaTime);
    ocean->Update(deltaTime);
    camera->FollowPlayer(player->position, player->front, deltaTime);
}

void Game::Render() {
    // Clear buffers with underwater color
    glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    shader->use();
    
    // Set up view and projection matrices
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom),
                                           (float)screenWidth / (float)screenHeight,
                                           0.1f, 500.0f);
    
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    
    // Set lighting
    glm::vec3 lightPos(0.0f, 50.0f, 0.0f); // Sun position above water
    shader->setVec3("lightPos", lightPos);
    shader->setVec3("viewPos", camera->Position);
    shader->setVec3("lightColor", 1.0f, 0.95f, 0.8f); // Warm sunlight
    
    // Set time for animations
    shader->setFloat("time", glfwGetTime());
    
    // Draw ocean environment
    ocean->Draw(*shader);
    
    // Draw player
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
