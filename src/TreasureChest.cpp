#include "TreasureChest.h"
#include <cmath>
#include <random>
#include <iostream>

TreasureChest::TreasureChest(Model* chest, Model* coin, glm::vec3 pos)
    : chestModel(chest), coinModel(coin), position(pos), scale(15.0f),  // MUCH bigger chest (was 5.0)
      collisionRadius(3.5f),  // Even tighter hitbox - almost touching (was 5.0)
      isOpened(false),
      openAnimation(0.0f),
      openSpeed(1.0f),
      coinSpawnTimer(0.0f),
      coinSpawnInterval(0.03f), // Spawn coin every 0.03 seconds (faster burst)
      coinsSpawned(0),
      maxCoins(50) { // More coins for impressive effect
    
    std::cout << "TreasureChest created. coinModel pointer: " << (void*)coinModel << std::endl;
    if (!coinModel) {
        std::cout << "WARNING: coinModel is NULL!" << std::endl;
    }
}

void TreasureChest::Update(float deltaTime) {
    // Spawn coins when chest is first opened
    if (isOpened && coinsSpawned < maxCoins) {
        coinSpawnTimer += deltaTime;
        if (coinSpawnTimer >= coinSpawnInterval) {
            if (coinsSpawned == 0) {
                std::cout << "Starting to spawn coins! deltaTime: " << deltaTime << std::endl;
            }
            SpawnCoin();
            coinSpawnTimer = 0.0f;
        }
    }
    
    // Always update coin physics
    UpdateCoins(deltaTime);
}

void TreasureChest::Draw(Shader& shader) {
    shader.setBool("isGlowing", false);
    shader.setBool("isCave", false);
    
    // Draw chest with opening animation
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, position);
    modelMat = glm::rotate(modelMat, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate to face forward
    modelMat = glm::scale(modelMat, glm::vec3(scale));
    
    // Could add lid rotation here based on openAnimation if needed
    // For now, just draw the chest normally
    
    shader.setMat4("model", modelMat);
    chestModel->Draw(shader);
    
    // Draw coins
    DrawCoins(shader);
}

bool TreasureChest::CheckCollision(const glm::vec3& playerPos, float playerRadius) {
    if (isOpened) return false; // Already opened
    
    float distance = glm::length(playerPos - position);
    return distance < (collisionRadius + playerRadius);
}

void TreasureChest::Open() {
    if (!isOpened) {
        isOpened = true;
        std::cout << "=== TREASURE CHEST OPENED! ===" << std::endl;
        std::cout << "Spawning " << maxCoins << " coins..." << std::endl;
    }
}

void TreasureChest::SpawnCoin() {
    std::cout << "SpawnCoin() called! Count: " << coinsSpawned + 1 << std::endl;
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    static std::uniform_real_distribution<float> speedDist(8.0f, 20.0f); // Faster horizontal speed
    static std::uniform_real_distribution<float> upSpeedDist(15.0f, 25.0f); // Much higher vertical speed
    static std::uniform_real_distribution<float> rotSpeedDist(360.0f, 1080.0f); // Faster spinning
    
    Coin coin;
    coin.position = position + glm::vec3(0.0f, 2.0f, 0.0f); // Lower - just above the chest opening
    
    // Random outward velocity with more dramatic spread
    float angle = angleDist(gen);
    float speed = speedDist(gen);
    coin.velocity = glm::vec3(
        cos(angle) * speed,
        upSpeedDist(gen), // Much higher upward velocity for dramatic fountain effect
        sin(angle) * speed
    );
    
    coin.rotationY = angleDist(gen) * 57.2958f; // Random starting rotation
    coin.rotationSpeed = rotSpeedDist(gen);
    coin.lifetime = 0.0f;
    coin.maxLifetime = 5.0f; // Coins last 5 seconds (even longer)
    coin.active = true;
    
    coins.push_back(coin);
    coinsSpawned++;
    
    // Debug output for first few coins
    if (coinsSpawned <= 5) {
        std::cout << "Spawned coin #" << coinsSpawned << " at position (" 
                  << coin.position.x << ", " << coin.position.y << ", " << coin.position.z << ")"
                  << " velocity (" << coin.velocity.x << ", " << coin.velocity.y << ", " << coin.velocity.z << ")" << std::endl;
    }
}

void TreasureChest::UpdateCoins(float deltaTime) {
    for (auto& coin : coins) {
        if (!coin.active) continue;
        
        // Update lifetime
        coin.lifetime += deltaTime;
        if (coin.lifetime >= coin.maxLifetime) {
            coin.active = false;
            continue;
        }
        
        // Apply gravity (underwater gravity - slower)
        coin.velocity.y -= 6.0f * deltaTime; // Reduced gravity for underwater effect
        
        // Apply slight water resistance (damping)
        coin.velocity *= 0.98f;
        
        // Update position
        coin.position += coin.velocity * deltaTime;
        
        // Update rotation (spinning)
        coin.rotationY += coin.rotationSpeed * deltaTime;
        
        // Floor collision (simple bounce)
        if (coin.position.y < position.y - 2.0f) {
            coin.position.y = position.y - 2.0f;
            coin.velocity.y *= -0.4f; // Bounce with more damping
            coin.velocity.x *= 0.7f; // Lose horizontal speed on bounce
            coin.velocity.z *= 0.7f;
        }
    }
}

void TreasureChest::DrawCoins(Shader& shader) {
    if (!coinModel) {
        std::cout << "ERROR: coinModel is null!" << std::endl;
        return;
    }
    
    int activeCoins = 0;
    for (const auto& coin : coins) {
        if (!coin.active) continue;
        activeCoins++;
        
        // Make coins glow like they're gold
        shader.setBool("isGlowing", true);
        
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, coin.position);
        modelMat = glm::rotate(modelMat, glm::radians(coin.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMat = glm::rotate(modelMat, glm::radians(coin.rotationY * 0.5f), glm::vec3(1.0f, 0.0f, 0.0f)); // Add X rotation
        modelMat = glm::scale(modelMat, glm::vec3(2.0f)); // MUCH bigger coins (was 0.5)
        
        shader.setMat4("model", modelMat);
        coinModel->Draw(shader);
    }
    
    // Debug: print active coin count occasionally
    static int frameCount = 0;
    if (frameCount++ % 60 == 0) {
        std::cout << "Active coins: " << activeCoins << " / " << coins.size() << " spawned: " << coinsSpawned << std::endl;
    }
    
    shader.setBool("isGlowing", false);
}
