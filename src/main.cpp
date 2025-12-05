#include "Game.h"
#include <iostream>

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "    Fish Story 2 - The Greatest Game    " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Create game instance
    Game game(1280, 720);
    
    // Initialize the game
    if (!game.Initialize()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return -1;
    }
    
    // Run the game loop
    game.Run();
    
    return 0;
}
