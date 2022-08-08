#include <iostream>

#include "game.hpp"
#include "heuristics.hpp"
#include "strategies/ExpectimaxDepthStrategy.hpp"
#include "strategies/ExpectimaxProbabilityStrategy.hpp"
#include "strategies/MinimaxStrategy.hpp"
#include "strategies/MonteCarloPlayer.hpp"
#include "strategies/OrderedPlayer.hpp"
#include "strategies/RandomPlayer.hpp"
#include "strategies/RandomTrialsStrategy.hpp"
#include "strategies/RotatingPlayer.hpp"
#include "strategies/SpamCornerPlayer.hpp"
#include "strategies/UserPlayer.hpp"

// range for recording results
constexpr int MIN_TILE = 3;  // 2^3 = 8
constexpr int MAX_TILE = 16; // 2^16 = 65536; theoretically 2^17 is possible but it's practically not

int results[MAX_TILE + 1];  // counts how many games reached this tile (or higher)
long long score_total = 0;
int move_total = 0;

const int play_game(Strategy& player, const bool print_results) {
    std::string record = "";
    const board_t board = player.simulator.play(player, record);

    const int fours = count_fours(record);
    move_total += count_moves_made(board, fours);

    const int score = actual_score(board, fours);
    score_total += score;
    if (print_results) {
        std::cout << "Fours: " << fours << std::endl;
        std::cout << "Score: " << score << std::endl;
        std::cout << "Record: " << record << std::endl;
    }
    return get_max_tile(board);
}

void test_player(Strategy& player, const int games) {
    std::fill(results, results + MAX_TILE + 1, 0);
    score_total = move_total = 0;

    const long long start_time = get_current_time_ms();
    for (int i = 1; i <= games; ++i) {
        const int max_tile = play_game(player, games <= 50);
        ++results[max_tile];  // suffix sum type thing
    }
    const long long end_time = get_current_time_ms();
    float time_taken = (end_time - start_time) / 1000.0;

    std::cout << "Playing " << games << " games took " << time_taken << " seconds (" << time_taken / games << " seconds per game)\n";

    for (int i = MAX_TILE - 1; i >= 0; --i) results[i] += results[i + 1];
    for (int i = MIN_TILE; i <= MAX_TILE; ++i) {
        std::cout << i << ' ' << results[i] << " (" << 100.0 * results[i] / games << ')' << std::endl;
    }
    std::cout << "Average score: " << score_total * 1.0 / games << std::endl;
    std::cout << "Total moves: " << move_total << std::endl;
}

//SpamCornerPlayer spam_corner_player{};
//MinimaxStrategy minimax_strategy(0, heuristics::strict_wall_heuristic);
//ExpectimaxDepthStrategy expectimax_depth_strategy(0, heuristics::monotonicity_heuristic);
ExpectimaxProbabilityStrategy expectimax_probability_strategy(0.005, heuristics::monotonicity_heuristic);

int main() {
    //const auto player = std::make_unique<RandomPlayer>();
    //test_player(*player, int(1e6));

    //test_player(spam_corner_player, int(1e5));  // spam_corner is the most efficient blind strategy

    std::string record = "";

    //UserPlayer user_player{};
    //user_player.simulator.play(user_player, record);

    //minimax_strategy.play_slow(minimax_strategy, record);
    //test_player(minimax_strategy, 5);

    //expectimax_depth_strategy.simulator.play_slow(expectimax_depth_strategy, record, [](const board_t board) -> void {
    //    std::cout << "Monotonicity: " << heuristics::monotonicity_heuristic(board) << std::endl;
    //    std::cout << "Duplicate: " << heuristics::_duplicate_score(board) << std::endl;
    //});
    //test_player(expectimax_depth_strategy, 10);

    //expectimax_probability_strategy.simulator.play_slow(expectimax_probability_strategy, record);
    test_player(expectimax_probability_strategy, 10);
}

