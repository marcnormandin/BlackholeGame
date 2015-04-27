#include <iostream>
#include <stdexcept>
#include "game.h"

int main(int argc, char *argv[]) {
    try {
        Game game;
        game.run();
    }
    catch(const std::exception& e) {
        std::cerr << e.what();
    }
    
    return 0;
}
