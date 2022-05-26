#include <fstream>
#include <iostream>
#include <future>

#include "game.cpp"
#include "heuristics.hpp"
#include "strategies/Strategy.hpp"
#include "strategies/ExpectimaxStrategy.hpp"
#include "strategies/MinimaxStrategy.hpp"
#include "strategies/MonteCarloPlayer.hpp"
#include "strategies/OrderedPlayer.hpp"
#include "strategies/RandomPlayer.hpp"
#include "strategies/RandomTrialsStrategy.hpp"
#include "strategies/RotatingPlayer.hpp"
#include "strategies/SpamCornerPlayer.hpp"
#include "strategies/UserPlayer.hpp"


constexpr int MIN_TILE = 3;   // getting 2^3 should always be guaranteed
constexpr int MAX_TILE = 18;  // 2^17 is largest possible tile

//constexpr int GAMES[5] = {500, 2000, 10000, 200000, 500000};
//constexpr int MAX_DEPTH = 5;
//constexpr int TRIALS[MAX_DEPTH + 1] = {0, 10, 10, 9, 7, 4};

constexpr int GAMES[5] = {5, 20, 100, 200, 500};
constexpr int MAX_DEPTH = 4;
constexpr int TRIALS[MAX_DEPTH + 1] = {0, 4, 4, 3, 2};

constexpr int THREADS = 8;

std::atomic<int> results[MAX_TILE];

const int play_game(Strategy& player) {
    int fours = 0;
    const board_t board = game::play(player, fours);
    return get_max_tile(board);
}

void write_headings(std::ofstream& fout) {
    assert(fout.is_open());  // might need to create the /results directory if this doesn't work
    fout << "Strategy,Games,Time Taken,Computation Time,Parallel";
    for (int i=MIN_TILE; i<MAX_TILE; ++i) {
        fout << ',' << (1 << i);
    }
    fout << std::endl;
}

void save_results(std::ofstream& fout, const std::string& player_name, const int games, const float time_taken, const float computation_time, const bool parallelize = false) {
    assert(fout.is_open());
    fout << player_name << ',' << games << ',' << time_taken << ',' << computation_time << ',' << (parallelize ? "true" : "false");
    for (int i=MIN_TILE; i<MAX_TILE; ++i) {
        fout << ',' << results[i];
    }
    fout << std::endl;
}

std::future<long long> futures[THREADS];
std::atomic<int> games_remaining;

long long test_player_thread(const std::unique_ptr<Strategy> player) {  // this function should own the player pointer
    const long long start_time = get_current_time_ms();
    while (games_remaining-- > 0) {
        const int max_tile = play_game(*player);
        ++results[max_tile];  // suffix sum type thing
        player->reset();
    }
    const long long end_time = get_current_time_ms();
    return end_time - start_time;
}

void test_player(std::ofstream& fout, const std::string& player_name, std::unique_ptr<Strategy> player, const int games, const bool print_progress) {
    std::cout << "\n\nTesting " << player_name << " player..." << std::endl;
    std::fill(results, results+MAX_TILE, 0);

    games_remaining.store(games);

    const long long start_time = get_current_time_ms();
    for (int i=0; i<THREADS; i++) {
        // give the Strategy pointer ownership to test_player_thread
        // move the player pointer last so that it can be cloned first
        futures[i] = std::async(test_player_thread, i+1 == THREADS ? std::move(player) : player->clone());
    }

    long long computation_time_ms = 0;
    for (int i=0; i<THREADS; i++) {
        computation_time_ms += futures[i].get();
    }
    const long long end_time = get_current_time_ms();

    const float computation_time = computation_time_ms / 1000.0;
    const float time_taken = (end_time - start_time) / 1000.0;
    std::cout << "Playing " << games << " games took " << time_taken << " seconds (" << time_taken / games << " seconds per game, computation time " << computation_time << ")\n";

    for (int i=MAX_TILE-2; i>=0; --i) {
        results[i] += results[i+1];
    }
    for (int i=MIN_TILE; i<MAX_TILE; ++i) {
        std::cout << i << ' ' << results[i] << " (" << 100.0 * results[i] / games << ')' << std::endl;
    }
    save_results(fout, player_name, games, time_taken, computation_time);
}

void test_single_player(const std::string& player_name, std::unique_ptr<Strategy> player, const int games,
                        const bool parallelize = false, const bool print_progress = false) {
    std::ofstream fout("results/" + player_name + ".csv");  // put results into a CSV for later collation
    write_headings(fout);
    test_player(fout, player_name, std::move(player), games, print_progress);  // give ownership of Strategy pointer
    fout.close();
}

void test_heuristic(const std::string& name, heuristic_t heuristic) {
    std::ofstream fout("results/" + name + "-rnd_t.csv");  // put results into a CSV for later collation
    write_headings(fout);
    for (int depth = 1; depth <= MAX_DEPTH; depth++) {
        for (int trials = 1; trials <= TRIALS[depth]; trials++) {
            const std::string player_name = name + "-rnd_t(d=" + std::to_string(depth) + " t=" + std::to_string(trials) + ")";

            const int order = depth * 10 + trials;
            const int speed = order <= 15 ? 3 : (order <= 24 || order % 10 == 1 ? 2 : (order <= 33 ? 1 : 0));

            test_player(fout, player_name, std::make_unique<RandomTrialsStrategy>(depth, trials, heuristic), GAMES[speed], speed == 0);
        }
    }
    fout.close();

    fout = std::ofstream("results/" + name + "-mnmx.csv");
    write_headings(fout);
    for (int depth = 1; depth < MAX_DEPTH; depth++) {
        const std::string player_name = name + "-mnmx(d=" + std::to_string(depth) + ")";

        const int speed = std::max(0, 3 - depth);

        test_player(fout, player_name, std::make_unique<MinimaxStrategy>(depth, heuristic), GAMES[speed], speed == 0);
    }
    fout.close();

    fout = std::ofstream("results/" + name + "-expmx.csv");
    write_headings(fout);
    for (int depth = 1; depth < MAX_DEPTH; depth++) {
        const std::string player_name = name + "-expmx(d=" + std::to_string(depth) + ")";

        const int speed = std::max(0, 3 - depth);

        test_player(fout, player_name, std::make_unique<ExpectimaxStrategy>(depth, heuristic), GAMES[speed], speed == 0);
    }
    fout.close();
}

void test_monte_carlo_strategy() {
    std::ofstream fout("results/monte_carlo.csv");
    write_headings(fout);
    for (int trials=100; trials<=2000; trials+=100) {
        test_player(fout, "monte_carlo (t=" + std::to_string(trials) + ")", std::make_unique<MonteCarloPlayer>(trials), GAMES[0], true);
    }
    fout.close();
}

int main() {
    game::init();

    //while (1) {
    //    board_t b;
    //    std::cin >> b;
    //    print_board(b);
    //}
    //
    //return 0;

    test_single_player("random", std::make_unique<RandomPlayer>(), GAMES[4]);
    test_single_player("spam_corner", std::make_unique<SpamCornerPlayer>(), GAMES[4]);
    test_single_player("ordered", std::make_unique<OrderedPlayer>(), GAMES[4]);
    test_single_player("rotating", std::make_unique<RotatingPlayer>(), GAMES[4]);

    test_heuristic("merge", heuristics::merge_heuristic);
    test_heuristic("score", heuristics::score_heuristic);
    test_heuristic("corner", heuristics::corner_heuristic);
    test_monte_carlo_strategy();


    std::cout << "Done!" << std::endl;
}

