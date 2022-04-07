#include <iostream>

#include "../game/game.cpp"
#include "ask_user.cpp"
#include "random.cpp"

int main() {
    game::init();

    //game::play(user_player::player);
    
    for (int i=0; i<1000; ++i) {
        const board_t board = game::play(random_player::player);
        std::cout << get_max_tile(board) << '\n';
    }
}
