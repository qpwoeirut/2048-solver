#include <iostream>

#include "game/game.cpp"
#include "solvers/ask_user.cpp"
#include "solvers/corner.cpp"
#include "solvers/ordered.cpp"
#include "solvers/random.cpp"

const int GAMES = 5e5;
const int MAX_TILE = 18;

int results[MAX_TILE];
void test_player(const std::string strategy, int (*player)(board_t)) {
    std::cout << "Testing " << strategy << " strategy..." << std::endl;
    std::fill(results, results+MAX_TILE, 0);

    const long long start_time = get_current_time_ms();
    for (int i=0; i<GAMES; ++i) {
        const board_t board = game::play(random_player::player);
        const int max_tile = get_max_tile(board);
        ++results[max_tile];  // suffix sum type thing
    }
    const long long end_time = get_current_time_ms();
    for (int i=MAX_TILE-2; i>=0; --i) {
        results[i] += results[i+1];
    }

    float time_taken = (end_time - start_time) / 1000.0;

    std::cout << "Playing " << GAMES << " games took " << time_taken << " seconds.\n";

    for (int i=0; i<MAX_TILE; ++i) {
        std::cout << i << ' ' << results[i] << std::endl;
    }
}

int main() {
    game::init();

    //game::play(user_player::player);
    
    test_player("corner", corner_player::player);
    test_player("ordered", ordered_player::player);
    test_player("random", random_player::player);
}
