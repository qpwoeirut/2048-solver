#include <fstream>
#include <iostream>
#include <future>

#include "game.hpp"
#include "heuristics.hpp"
#include "strategies/Strategy.hpp"
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
constexpr int MIN_TILE = 3;   // getting 2^3 should always be guaranteed
constexpr int MAX_TILE = 16;  // 2^17 is largest possible tile, but it's practically impossible

constexpr int GAMES[5] = {500, 2000, 10000, 200000, 500000};
constexpr int MAX_DEPTH = 5;
constexpr int TRIALS[MAX_DEPTH + 1] = {0, 10, 10, 10, 9, 6};

// small testing parameters for local testing to catch bugs
//constexpr int GAMES[5] = {4, 10, 25, 50, 100};
//constexpr int MAX_DEPTH = 4;
//constexpr int TRIALS[MAX_DEPTH + 1] = {0, 5, 5, 4, 3};

constexpr int THREADS = 4;

std::atomic<int> results[MAX_TILE + 1];  // counts how many games reached this tile (or higher)
int moves[GAMES[4]];  // each index is only modified by one thread so atomic isn't necessary
int scores[GAMES[4]];

long long calculate_total(const int arr[], const int n) {
    return std::accumulate(arr, arr + n, 0LL);
}

float calculate_median(const int arr[], const int n) {  // if n is even, returns mean of two middle elements
    std::vector<int> tmp(arr, arr + n);
    sort(tmp.begin(), tmp.end());
    return (n & 1) ? tmp[n / 2] : (tmp[(n - 1) / 2] + tmp[n / 2]) / 2.0;
}

void write_headings(std::ofstream& fout) {
    assert(fout.is_open());  // might need to create the /results directory if this doesn't work
    fout << "Strategy,Games,Time Taken,Computation Time";
    for (int i = MIN_TILE; i <= MAX_TILE; ++i) {
        fout << ',' << (1 << i);
    }
    fout << ",Total Score,Median Score,Total Moves,Median Moves";
    fout << std::endl;
}

void save_results(std::ofstream& fout, const std::string& player_name, const int games, const float time_taken,
                  const float computation_time) {
    assert(fout.is_open());
    fout << player_name << ',' << games << ',' << time_taken << ',' << computation_time;
    for (int i = MIN_TILE; i <= MAX_TILE; ++i) {
        fout << ',' << results[i];
    }
    fout << ',' << calculate_total(scores, games) << ',' << calculate_median(scores, games) << ',' << calculate_total(moves, games) << ',' << calculate_median(moves, games);
    fout << std::endl;
}

std::future<long long> futures[THREADS];
std::atomic<int> games_remaining;

long long test_player_thread(const std::unique_ptr<Strategy> player) {  // this function should own the player pointer
    const long long start_time = get_current_time_ms();
    int game_idx = --games_remaining;
    while (game_idx >= 0) {
        std::string record = "";
        const board_t board = player->simulator.play(*player, record);
        player->reset();
        const int max_tile = get_max_tile(board);
        ++results[max_tile];  // suffix sum type thing

        const int fours = count_fours(record);
        moves[game_idx] = count_moves_made(board, fours);
        scores[game_idx] = actual_score(board, fours);

        game_idx = --games_remaining;  // games_remaining will end up negative, but that's fine
    }
    const long long end_time = get_current_time_ms();
    return end_time - start_time;
}

void test_player(std::ofstream& fout, const std::string& player_name, std::unique_ptr<Strategy> player, const int games) {
    std::cout << "\n\nTesting " << player_name << " player..." << std::endl;
    std::fill(results, results + MAX_TILE + 1, 0);

    games_remaining.store(games);

    const long long start_time = get_current_time_ms();
    for (int i = 1; i < THREADS; i++) {
        // give the Strategy pointer ownership to test_player_thread
        // move the player pointer last so that it can be cloned first
        futures[i] = std::async(test_player_thread, player->clone());
    }
    futures[0] = std::async(test_player_thread, std::move(player));

    long long computation_time_ms = 0;
    for (int i = 0; i < THREADS; i++) {
        computation_time_ms += futures[i].get();
    }
    const long long end_time = get_current_time_ms();

    const float computation_time = computation_time_ms / 1000.0;
    const float time_taken = (end_time - start_time) / 1000.0;
    std::cout << "Playing " << games << " games took " << time_taken << " seconds (" << time_taken / games << " seconds per game, computation time " << computation_time << ")\n";

    for (int i = MAX_TILE - 1; i >= 0; --i) results[i] += results[i + 1];
    for (int i = MIN_TILE; i <= MAX_TILE; ++i) {
        std::cout << i << ' ' << results[i] << " (" << 100.0 * results[i] / games << ')' << std::endl;
    }
    save_results(fout, player_name, games, time_taken, computation_time);
}

void test_single_player(const std::string& player_name, std::unique_ptr<Strategy> player, const int games) {
    std::ofstream fout("results/" + player_name + ".csv");  // put results into a CSV for later collation
    write_headings(fout);
    test_player(fout, player_name, std::move(player), games);  // give ownership of Strategy pointer
    fout.close();
}

void test_heuristic(const std::string& name, heuristic_t heuristic) {
    std::ofstream fout("results/" + name + "-rnd_t.csv");  // put results into a CSV for later collation
    write_headings(fout);
    for (int depth = 1; depth <= MAX_DEPTH; depth++) {
        for (int trials = 1; trials <= TRIALS[depth]; trials++) {
            const std::string player_name = name + "-rnd_t(d=" + std::to_string(depth) + " t=" + std::to_string(trials) + ")";

            const int order = depth * 10 + trials;
            const int speed = order <= 21 ? 3 : (order <= 25 || order % 10 == 1 ? 2 : (order <= 33 ? 1 : 0));

            test_player(fout, player_name, std::make_unique<RandomTrialsStrategy>(depth, trials, heuristic), GAMES[speed]);
        }
    }
    fout.close();

    fout = std::ofstream("results/" + name + "-mnmx.csv");
    write_headings(fout);
    for (int depth = -1; depth <= MAX_DEPTH; depth++) {  // include depth=-1 and depth=0, which use depth picker
        const std::string player_name = name + "-mnmx(d=" + std::to_string(depth) + ")";
        const int speed = depth <= 0 || depth >= 4 ? 0 : 4 - depth;
        test_player(fout, player_name, std::make_unique<MinimaxStrategy>(depth, heuristic), GAMES[speed]);
    }
    fout.close();

    fout = std::ofstream("results/" + name + "-expmx.csv");
    write_headings(fout);

    // include depth=-1 and depth=0, which uses depth picker; d=MAX_DEPTH will take too long
    for (int depth = -1; depth < MAX_DEPTH; depth++) {
        const std::string player_name = name + "-expmx(d=" + std::to_string(depth) + ")";
        const int speed = depth <= 0 || depth >= 4 ? 0 : 4 - depth;
        test_player(fout, player_name, std::make_unique<ExpectimaxDepthStrategy>(depth, heuristic), GAMES[speed]);
    }
    for (double prob = 0.1; prob >= 0.001; prob /= 10) {
        const int speed = (prob >= 0.1) + (prob >= 0.01) + (prob >= 0.001);

        std::string player_name = name + "-expmx(p=" + std::to_string(prob * 5) + ")";
        test_player(fout, player_name, std::make_unique<ExpectimaxProbabilityStrategy>(prob * 5, heuristic), GAMES[speed]);

        player_name = name + "-expmx(p=" + std::to_string(prob) + ")";
        test_player(fout, player_name, std::make_unique<ExpectimaxProbabilityStrategy>(prob, heuristic), GAMES[speed]);
    }
    fout.close();
}

void test_monte_carlo_strategy() {
    std::ofstream fout("results/monte_carlo.csv");
    write_headings(fout);
    for (int trials = 100; trials <= 2500; trials += 100) {
        test_player(fout, "monte_carlo (t=" + std::to_string(trials) + ")", std::make_unique<MonteCarloPlayer>(trials), GAMES[0]);
    }
    fout.close();
}

void run_board_echo() {
    while (true) {
        board_t b;
        std::cin >> b;
        print_board(b);
    }
}

int main() {
    //run_board_echo(); return 0;

    //std::string record = "";
    //const auto player = std::make_unique<ExpectimaxProbabilityStrategy>(0.0001, heuristics::corner_heuristic);
    //player->simulator.play_slow(*player, record);
    //return 0;

    test_single_player("random", std::make_unique<RandomPlayer>(), GAMES[4]);
    test_single_player("spam_corner", std::make_unique<SpamCornerPlayer>(), GAMES[4]);
    test_single_player("ordered", std::make_unique<OrderedPlayer>(), GAMES[4]);
    test_single_player("rotating", std::make_unique<RotatingPlayer>(), GAMES[4]);

    test_heuristic("merge", heuristics::merge_heuristic);
    test_heuristic("score", heuristics::score_heuristic);
    test_heuristic("corner", heuristics::corner_heuristic);
    test_heuristic("full_wall", heuristics::full_wall_heuristic);
    test_heuristic("wall_gap", heuristics::wall_gap_heuristic);
    test_monte_carlo_strategy();


    std::cout << "Done!" << std::endl;
}
