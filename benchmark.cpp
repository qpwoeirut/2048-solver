#include <iostream>

#include "game.cpp"
#include "heuristics.cpp"
#include "strategies/blind/spam_corner.cpp"
#include "strategies/expectimax.cpp"
#include "strategies/minimax.cpp"

constexpr int MIN_TILE = 3;
constexpr int MAX_TILE = 18;

int results[MAX_TILE];
long long score_total = 0;
int move_total = 0;

const int play_game(const player_t player) {
    int fours = 0;
    const board_t board = game::play(player, fours);

    move_total += (board_sum(board) >> 1) - fours - 2;
    // every move, a 2 or 4 tile spawns, so we can calculate move count by board sum
    // the -2 is because the board starts with two tiles

    const int score = heuristics::score_heuristic(board) - 4 * fours;
    score_total += score;
    std::cout << "Score: " << score << std::endl;
    return get_max_tile(board);
}

void test_player(const player_t player, const int games) {
    std::fill(results, results+MAX_TILE, 0);

    const long long start_time = get_current_time_ms();
    for (int i=1; i<=games; ++i) {
        const int max_tile = play_game(player);
        ++results[max_tile];  // suffix sum type thing
    }
    const long long end_time = get_current_time_ms();
    float time_taken = (end_time - start_time) / 1000.0;

    std::cout << "Playing " << games << " games took " << time_taken << " seconds (" << time_taken / games << " seconds per game)\n";

    for (int i=MAX_TILE-2; i>=0; --i) {
        results[i] += results[i+1];
    }
    for (int i=MIN_TILE; i<MAX_TILE; ++i) {
        std::cout << i << ' ' << results[i] << " (" << 100.0 * results[i] / games << ')' << std::endl;
    }
    std::cout << "Average score: " << score_total * 1.0 / games << std::endl;
    std::cout << "Total moves: " << move_total << std::endl;
}

int main() {
    game::init(8);  // make the game repeatable
    move_gen.seed(8);

    //test_player("spam_corner", spam_corner_strategy::player, int(1e5));  // spam_corner is the most efficient blind strategy

    int f = 0;

    minimax_strategy::init(0, heuristics::score_heuristic);
    //game::play_slow(minimax_strategy::player, f);
    test_player(minimax_strategy::player, 20);

//    expectimax_strategy::init(0, heuristics::full_wall_heuristic);
//    game::play_slow(expectimax_strategy::player, f);
//    test_player("full-wall-expmx", expectimax_strategy::player, 3);
}

/*
minimax depth=0, score heuristic
With cache, no pruning

Score: 7340
Score: 16312
Score: 14480
Score: 7320
Score: 7544
Score: 12412
Score: 27468
Score: 16212
Score: 14636
Score: 16220
Score: 7376
Score: 23292
Score: 27312
Score: 16016
Score: 15508
Score: 14840
Score: 13952
Score: 16120
Score: 14300
Score: 7224
Playing 20 games took 30.982 seconds (1.5491 seconds per game)
...
9 20 (100)
10 15 (75)
11 3 (15)
12 0 (0)
...
Average score: 14794.2
Total moves: 17247


With alpha-beta pruning, no cache
Playing 20 games took 8.402 seconds (0.4201 seconds per game)
*/

