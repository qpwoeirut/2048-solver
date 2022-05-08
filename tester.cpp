#include <fstream>
#include <iostream>
#include <future>
#include <vector>

#include "game/game.cpp"
#include "heuristics.cpp"
#include "strategies/ask_user.cpp"
#include "strategies/random.cpp"
#include "strategies/corner.cpp"
#include "strategies/ordered.cpp"
#include "strategies/rotating.cpp"
#include "strategies/minimax.cpp"
#include "strategies/expectimax.cpp"
#include "strategies/monte_carlo.cpp"

constexpr int MAX_DEPTH = 1;
constexpr int MAX_TRIALS = 2;

constexpr int MIN_TILE = 3;   // getting 2^3 should always be guaranteed
constexpr int MAX_TILE = 18;  // 2^17 is largest possible tile

//constexpr int GAMES[5] = {500, 2000, 10000, 200000, 500000};
constexpr int GAMES[5] = {5, 20, 100, 200, 500};
constexpr int MAX_THREADS = GAMES[0];  // this is already probably higher than ideal

int results[MAX_TILE];

const int play_game(const int (*player)(const board_t)) {
    const board_t board = game::play(player);
    return get_max_tile(board);
}

void write_headings(std::ofstream& fout) {
    assert(fout.is_open());  // might need to create the /results directory if this doesn't work
    fout << "Strategy,Games,Time,Parallel";
    for (int i=MIN_TILE; i<MAX_TILE; ++i) {
        fout << ',' << (1 << i);
    }
    fout << std::endl;
}

void save_results(std::ofstream& fout, const std::string& strategy, const int games, const float time_taken, const bool parallelize) {
    assert(fout.is_open());
    fout << strategy << ',' << games << ',' << time_taken << ',' << (parallelize ? "true" : "false");
    for (int i=MIN_TILE; i<MAX_TILE; ++i) {
        fout << ',' << results[i];
    }
    fout << std::endl;
}

std::future<int> futures[MAX_THREADS];

void test_player(std::ofstream& fout, const std::string& strategy, const int (*player)(board_t), const int games, const bool parallelize, const bool print_progress) {
    std::cout << "\n\nTesting " << strategy << " strategy..." << std::endl;
    std::fill(results, results+MAX_TILE, 0);

    const long long start_time = get_current_time_ms();
    if (parallelize) {
        assert(games <= MAX_THREADS);
        for (int i=0; i<games; ++i) {
            futures[i] = std::async(play_game, player);
        }
        for (int i=0; i<games; ++i) {
            ++results[futures[i].get()];
        }
    } else {
        for (int i=1; i<=games; ++i) {
            if (print_progress && i % 5 == 0) {
                std::cout << "Starting game #" << i << std::endl;
            }
            const int max_tile = play_game(player);
            ++results[max_tile];  // suffix sum type thing
        }
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
    save_results(fout, strategy, games, time_taken, parallelize);
}

void test_single_player(const std::string& strategy, const int (*player)(board_t), const int games,
                        const bool parallelize = false, const bool print_progress = false) {
    std::ofstream fout("results/" + strategy + ".csv");  // put results into a CSV for later collation
    write_headings(fout);
    test_player(fout, strategy, player, games, parallelize, print_progress);
    fout.close();
}

void test_heuristic(const std::string& heuristic_name, const long long (*heuristic)(const board_t)) {
    std::ofstream fout("results/" + heuristic + "-rnd_t.csv");  // put results into a CSV for later collation
    write_headings(fout);
    for (int depth = 1; depth <= MAX_DEPTH; depth++) {
        for (int trials = 1; trials <= (depth == MAX_DEPTH) ? MAX_TRIALS / 2 : MAX_TRIALS; trials++) {
            const std::strategy = heuristic + "-rnd_t(d=" + std::to_string(depth) + " t=" + std::to_string(trials) + ")";
            random_trials_player::init(depth, trials, heuristic);

            const int order = depth * 10 + trials;
            const int speed = order <= 15 ? 3 : (order <= 24 || order % 10 == 1 ? 2 : (order <= 33 ? 1 : 0));

            test_player(fout, strategy, random_trials_player::player, GAMES[speed], GAMES[speed] <= MAX_THREADS, speed == 0);
        }
    }
    fout.close();

    fout = std::ofstream("results/" + heuristic + "-mnmx.csv");
    write_headings(fout);
    for (int depth = 1; depth <= MAX_DEPTH; depth++) {
        const std::strategy = heuristic + "-mnmx(d=" + std::to_string(depth) + ")";
        minimax_player::init(depth, heuristic);

        const int speed = std::max(0, 3 - depth);
        test_player(fout, strategy, minimax_player::player, GAMES[speed], GAMES[speed] <= MAX_THREADS, speed == 0);
    }
    fout.close();

    fout = std::ofstream("results/" + heuristic + "-expmx.csv");
    write_headings(fout);
    for (int depth = 1; depth <= MAX_DEPTH; depth++) {
        const std::strategy = heuristic + "-expmx(d=" + std::to_string(depth) + ")";
        expectimax_player::init(depth, heuristic);

        const int speed = std::max(0, 3 - depth);
        test_player(fout, strategy, expectimax_player::player, GAMES[speed], GAMES[speed] <= MAX_THREADS, speed == 0);
    }
    fout.close();
}

void test_monte_carlo_player() {
    std::ofstream fout("results/monte_carlo.csv");
    write_headings(fout);
    for (int trials=100; trials<=2000; trials+=100) {
        monte_carlo_player::init(trials);
        test_player(fout, "monte_carlo (t=" + std::to_string(trials) + ")", monte_carlo_player::player, GAMES[0], true, true);
    }
    fout.close();
}

int main() {
    game::init();
    
    test_single_player("random", random_player::player, GAMES[4]);
    test_single_player("spam_corner", spam_corner_player::player, GAMES[4]);
    test_single_player("ordered", ordered_player::player, GAMES[4]);
    test_single_player("rotating", rotating_player::player, GAMES[4]);

    test_heuristic("merge", heuristics::merge_heuristic);
    test_heuristic("score", heuristics::score_heuristic);
    test_heuristic("corner", heuristics::corner_heuristic);
    test_monte_carlo_player();
}

