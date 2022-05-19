#include <fstream>
#include <iostream>
#include <future>
#include <vector>

#include "game.cpp"
#include "heuristics.cpp"
#include "strategies/ask_user.cpp"
#include "strategies/blind/random.cpp"
#include "strategies/blind/spam_corner.cpp"
#include "strategies/blind/ordered.cpp"
#include "strategies/blind/rotating.cpp"
#include "strategies/rand_trials.cpp"
#include "strategies/minimax.cpp"
#include "strategies/expectimax.cpp"
#include "strategies/monte_carlo.cpp"

constexpr int MAX_DEPTH = 5;
constexpr int TRIALS[MAX_DEPTH + 1] = {0, 10, 10, 9, 7, 4};

constexpr int MIN_TILE = 3;   // getting 2^3 should always be guaranteed
constexpr int MAX_TILE = 18;  // 2^17 is largest possible tile

constexpr int GAMES[5] = {500, 2000, 10000, 200000, 500000};
//constexpr int GAMES[5] = {5, 20, 100, 200, 500};
constexpr int MAX_THREADS = GAMES[0];  // this is already probably higher than ideal

int results[MAX_TILE];

const int play_game(const player_t player) {
    int fours = 0;
    const board_t board = game::play(player, fours);
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

void save_results(std::ofstream& fout, const std::string& player_name, const int games, const float time_taken, const bool parallelize) {
    assert(fout.is_open());
    fout << player_name << ',' << games << ',' << time_taken << ',' << (parallelize ? "true" : "false");
    for (int i=MIN_TILE; i<MAX_TILE; ++i) {
        fout << ',' << results[i];
    }
    fout << std::endl;
}

std::future<int> futures[MAX_THREADS];

void test_player(std::ofstream& fout, const std::string& player_name, const player_t player, const int games, const bool parallelize, const bool print_progress) {
    std::cout << "\n\nTesting " << player_name << " player..." << std::endl;
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
    float time_taken = (end_time - start_time) / 1000.0;

    std::cout << "Playing " << games << " games took " << time_taken << " seconds (" << time_taken / games << " seconds per game)\n";

    for (int i=MAX_TILE-2; i>=0; --i) {
        results[i] += results[i+1];
    }
    for (int i=MIN_TILE; i<MAX_TILE; ++i) {
        std::cout << i << ' ' << results[i] << " (" << 100.0 * results[i] / games << ')' << std::endl;
    }
    save_results(fout, player_name, games, time_taken, parallelize);
}

void test_single_player(const std::string& player_name, const player_t player, const int games,
                        const bool parallelize = false, const bool print_progress = false) {
    std::ofstream fout("results/" + player_name + ".csv");  // put results into a CSV for later collation
    write_headings(fout);
    test_player(fout, player_name, player, games, parallelize, print_progress);
    fout.close();
}

void test_heuristic(const std::string& name, heuristic_t heuristic) {
    std::ofstream fout("results/" + name + "-rnd_t.csv");  // put results into a CSV for later collation
    write_headings(fout);
    for (int depth = 1; depth <= MAX_DEPTH; depth++) {
        for (int trials = 1; trials <= TRIALS[depth]; trials++) {
            const std::string player_name = name + "-rnd_t(d=" + std::to_string(depth) + " t=" + std::to_string(trials) + ")";
            rand_trials_strategy::init(depth, trials, heuristic);

            const int order = depth * 10 + trials;
            const int speed = order <= 15 ? 3 : (order <= 24 || order % 10 == 1 ? 2 : (order <= 33 ? 1 : 0));

            test_player(fout, player_name, rand_trials_strategy::player, GAMES[speed], GAMES[speed] <= MAX_THREADS, speed == 0);
        }
    }
    fout.close();

    fout = std::ofstream("results/" + name + "-mnmx.csv");
    write_headings(fout);
    for (int depth = 1; depth < MAX_DEPTH; depth++) {
        const std::string player_name = name + "-mnmx(d=" + std::to_string(depth) + ")";
        minimax_strategy::init(depth, heuristic);

        const int speed = std::max(0, 3 - depth);
        test_player(fout, player_name, minimax_strategy::player, GAMES[speed], GAMES[speed] <= MAX_THREADS, speed == 0);
    }
    fout.close();

    fout = std::ofstream("results/" + name + "-expmx.csv");
    write_headings(fout);
    for (int depth = 1; depth < MAX_DEPTH; depth++) {
        const std::string player_name = name + "-expmx(d=" + std::to_string(depth) + ")";
        expectimax_strategy::init(depth, heuristic);

        const int speed = std::max(0, 3 - depth);
        test_player(fout, player_name, expectimax_strategy::player, GAMES[speed], GAMES[speed] <= MAX_THREADS, speed == 0);
    }
    fout.close();
}

void test_monte_carlo_strategy() {
    std::ofstream fout("results/monte_carlo.csv");
    write_headings(fout);
    for (int trials=100; trials<=2000; trials+=100) {
        monte_carlo_strategy::init(trials);
        test_player(fout, "monte_carlo (t=" + std::to_string(trials) + ")", monte_carlo_strategy::player, GAMES[0], true, true);
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

    test_single_player("random", random_strategy::player, GAMES[4]);
    test_single_player("spam_corner", spam_corner_strategy::player, GAMES[4]);
    test_single_player("ordered", ordered_strategy::player, GAMES[4]);
    test_single_player("rotating", rotating_strategy::player, GAMES[4]);

    test_heuristic("merge", heuristics::merge_heuristic);
    test_heuristic("score", heuristics::score_heuristic);
    test_heuristic("corner", heuristics::corner_heuristic);
    test_monte_carlo_strategy();
}

