#include <iostream>

#include "game.hpp"
#include "heuristics.hpp"
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
constexpr int MAX_TILE = 17;

int results[MAX_TILE];
long long score_total = 0;
int move_total = 0;

const int play_game(Strategy& player) {
    int fours = 0;
    const board_t board = player.simulator.play(player, fours);

    move_total += count_moves_made(board, fours);

    const int score = actual_score(board, fours);
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

SpamCornerPlayer spam_corner_player{};
//MinimaxStrategy minimax_strategy(0, heuristics::score_heuristic);
//ExpectimaxStrategy expectimax_strategy(-1, heuristics::corner_heuristic);
ExpectimaxStrategy expectimax_strategy(0, heuristics::distinct_heuristic);

int main() {
    //const auto player = std::make_unique<RandomPlayer>();
    //test_player(*player, int(1e6));

    //test_player(spam_corner_player, int(1e5));  // spam_corner is the most efficient blind strategy

    //int f = 0;

    //UserPlayer user_player{};
    //user_player.simulator.play(user_player, f);

    //minimax_strategy.play_slow(minimax_strategy, f);
    //test_player(minimax_strategy, 20);

    //expectimax_strategy.simulator.play_slow(expectimax_strategy, f);
    test_player(expectimax_strategy, 5);
}

