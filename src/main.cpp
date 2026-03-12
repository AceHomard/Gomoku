#include "Gomoku.hpp"
#include "Game/Game.hpp"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}