#include <iostream>

#include "game.cpp"
#include "heuristics.cpp"
#include "strategies/blind/spam_corner.cpp"
#include "strategies/expectimax.cpp"

constexpr int MIN_TILE = 3;
constexpr int MAX_TILE = 18;

int results[MAX_TILE];
long long score_total = 0;
int move_total = 0;

const int play_game(const int (*player)(const board_t)) {
    int fours = 0;
    const board_t board = game::play(player, move_total, fours);
    const int score = heuristics::score_heuristic(board) - 4 * fours;
    score_total += score;
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
    std::cout << "Average score: " << score_total / games << std::endl;
    std::cout << "Total moves: " << move_total << std::endl;
}

int main() {
    game::init();

     test_player("spam_corner", spam_corner_strategy::player, (int)(5e5));  // spam_corner is the most efficient blind strategy

//    expectimax_strategy::init(3, heuristics::corner_heuristic);
//    test_player("corner-expmx", expectimax_strategy::player, 50);
}

