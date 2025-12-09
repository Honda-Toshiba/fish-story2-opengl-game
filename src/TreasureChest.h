#ifndef TREASURECHEST_H
#define TREASURECHEST_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "Model.h"
#include "Shader.h"

// Coin particle for animation
struct Coin {
    glm::vec3 position;
    glm::vec3 velocity;
    float rotationY;
    float rotationSpeed;
    float lifetime;
    float maxLifetime;
    bool active;
};

class TreasureChest {
public:
    TreasureChest(Model* chestModel, Model* coinModel, glm::vec3 position);
    
    void Update(float deltaTime);
    void Draw(Shader& shader);
    
    // Returns true if player touches chest
    bool CheckCollision(const glm::vec3& playerPos, float playerRadius);
    
    bool IsOpened() const { return isOpened; }
    void Open(); // Trigger opening animation
    
private:
    Model* chestModel;
    Model* coinModel;
    glm::vec3 position;
    float scale;
    float collisionRadius;
    
    bool isOpened;
    float openAnimation; // 0.0 to 1.0
    float openSpeed;
    
    // Coin particles
    std::vector<Coin> coins;
    float coinSpawnTimer;
    float coinSpawnInterval;
    int coinsSpawned;
    int maxCoins;
    
    void SpawnCoin();
    void UpdateCoins(float deltaTime);
    void DrawCoins(Shader& shader);
};

#endif
