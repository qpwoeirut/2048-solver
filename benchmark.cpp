#include <iostream>

#include "game/game.cpp"
#include "strategies/blind/spam_corner.cpp"

constexpr int MIN_TILE = 3;
constexpr int MAX_TILE = 18;

int results[MAX_TILE];

const int play_game(const int (*player)(const board_t)) {
    const board_t board = game::play(player);
    return get_max_tile(board);
}

void test_player(const std::string& strategy, const int (*player)(board_t), const int games) {
    std::fill(results, results+MAX_TILE, 0);

    const long long start_time = get_current_time_ms();
    for (int i=1; i<=games; ++i) {
        const int max_tile = play_game(player);
        ++results[max_tile];  // suffix sum type thing
    }
    const long long end_time = get_current_time_ms();
    for (int i=MAX_TILE-2; i>=0; --i) {
        results[i] += results[i+1];
    }

    float time_taken = (end_time - start_time) / 1000.0;

    std::cout << "Playing " << games << " games took " << time_taken << " seconds (" << time_taken / games << " seconds per game)\n";

    for (int i=MIN_TILE; i<MAX_TILE; ++i) {
        std::cout << i << ' ' << results[i] << " (" << 100.0 * results[i] / games << ')' << std::endl;
    }
}

int main() {
    game::init();

    test_player("spam_corner", spam_corner_player::player, (int)(5e5));  // spam_corner is the most efficient blind strategy
}

