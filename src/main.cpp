#include "Game.h"
#include "GameLevel2.h"
#include <iostream>

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "    Fish Story 2 - The Greatest Game    " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Level selection
    std::cout << "Select Level:" << std::endl;
    std::cout << "  1 - Open Ocean (Level 1)" << std::endl;
    std::cout << "  2 - Dark Cave (Level 2)" << std::endl;
    std::cout << "Enter choice (1 or 2): ";
    
    int choice;
    std::cin >> choice;
    
    if (choice == 2) {
        // Run Level 2
        std::cout << "\nStarting Level 2..." << std::endl;
        GameLevel2 game(1280, 720);
        
        if (!game.Initialize()) {
            std::cerr << "Failed to initialize Level 2!" << std::endl;
            return -1;
        }
        
        game.Run();
    } else {
        // Run Level 1 (default)
        std::cout << "\nStarting Level 1..." << std::endl;
        bool shouldTransition = false;
        
        {
            // Scope block to ensure Game is destroyed before Level 2 starts
            Game game(1280, 720);
            
            if (!game.Initialize()) {
                std::cerr << "Failed to initialize game!" << std::endl;
                return -1;
            }
            
            game.Run();
            shouldTransition = game.ShouldTransitionToLevel2();
        } // Game destructor called here, window destroyed
        
        // Check if we should transition to Level 2
        if (shouldTransition) {
            std::cout << "\n\nTransitioning to Level 2..." << std::endl;
            GameLevel2 level2(1280, 720);
            
            if (!level2.Initialize()) {
                std::cerr << "Failed to initialize Level 2!" << std::endl;
                return -1;
            }
            
            level2.Run();
        }
    }
    
    // Cleanup GLFW at the very end
    glfwTerminate();
    
    return 0;
}
