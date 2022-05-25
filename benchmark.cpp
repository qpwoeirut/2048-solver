#include <iostream>

#include "game.cpp"
#include "heuristics.cpp"
#include "strategies/ExpectimaxStrategy.hpp"
#include "strategies/MinimaxStrategy.hpp"
#include "strategies/MonteCarloPlayer.hpp"
#include "strategies/OrderedPlayer.hpp"
#include "strategies/RandomPlayer.hpp"
#include "strategies/RandomTrialsStrategy.hpp"
#include "strategies/RotatingPlayer.hpp"
#include "strategies/SpamCornerPlayer.hpp"
#include "strategies/UserPlayer.hpp"

constexpr int MIN_TILE = 3;
constexpr int MAX_TILE = 18;

int results[MAX_TILE];
long long score_total = 0;
int move_total = 0;

const int play_game(Strategy& player) {
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

void test_player(Strategy& player, const int games) {
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

//MinimaxStrategy minimax_strategy(0, heuristics::score_heuristic);
ExpectimaxStrategy expectimax_strategy(-1, heuristics::corner_heuristic);

int main() {
    game::init(8);  // make the game repeatable
    move_gen.seed(8);

    //test_player("spam_corner", spam_corner_strategy::player, int(1e5));  // spam_corner is the most efficient blind strategy

    int f = 0;

    //UserPlayer user_player();
    //game::play(user_player, f);

    //game::play_slow(minimax_strategy, f);
    //test_player(minimax_strategy, 20);

    //game::play_slow(expectimax_strategy, f);
    test_player(expectimax_strategy, 1);
}

