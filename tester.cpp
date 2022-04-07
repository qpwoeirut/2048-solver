#include <iostream>

#include "game/game.cpp"
#include "solvers/ask_user.cpp"
#include "solvers/random.cpp"

const int MAX_TILE = 18;

int results[MAX_TILE];
void test_player(int (*player)(board_t)) {
    std::fill(results, results+MAX_TILE, 0);
    for (int i=0; i<100000; ++i) {  // play 1e5 games
        const board_t board = game::play(random_player::player);
        const int max_tile = get_max_tile(board);
        ++results[max_tile];  // suffix sum type thing
    }
    for (int i=MAX_TILE-2; i>=0; --i) {
        results[i] += results[i+1];
    }
}

int main() {
    game::init();

    //game::play(user_player::player);
    
    test_player(random_player::player);
    for (int i=0; i<MAX_TILE; ++i) {
        std::cout << i << ' ' << results[i] << std::endl;
    }
}
