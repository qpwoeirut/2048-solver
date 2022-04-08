#include <fstream>
#include <iostream>
#include <future>
#include <vector>

#include "game/game.cpp"
#include "solvers/ask_user.cpp"
#include "solvers/random.cpp"
#include "solvers/corner.cpp"
#include "solvers/ordered.cpp"
#include "solvers/merge.cpp"

std::ofstream fout("results.csv");  // put results into a CSV

constexpr int MIN_TILE = 3;   // getting 2^3 should always be guaranteed
constexpr int MAX_TILE = 18;  // 2^17 is largest possible tile

constexpr int GAMES[5] = {100, 1000, 5000, 50000, 200000};
constexpr int MAX_THREADS = GAMES[2];

std::future<int> futures[MAX_THREADS];
int results[MAX_TILE];

int play_game(const int (*player)(const board_t)) {
    const board_t board = game::play(player);
    return get_max_tile(board);
}

void test_player(const std::string strategy, const int (*player)(board_t), const int games,
                 const bool parallelize = false, const bool print_progress = false) {
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

    fout << strategy << ',' << games << ',' << time_taken;
    for (int i=MIN_TILE; i<MAX_TILE; ++i) {
        fout << ',' << 100.0 * results[i] / games;
    }
    fout << '\n';
}

int main() {
    fout << "Strategy,Games,Time";
    for (int i=MIN_TILE; i<MAX_TILE; ++i) {
        fout << ',' << (1 << i);
    }
    fout << '\n';

    game::init();

    //game::play_slow(merge_player::player);

    //game::play(user_player::player);
    
    test_player("random", random_player::player, GAMES[4]);
    test_player("corner", corner_player::player, GAMES[4]);
    test_player("ordered", ordered_player::player, GAMES[4]);
    for (int depth=1; depth<=4; ++depth) {
        for (int trials=1; trials<=10 - depth; ++trials) {
            const std::string strategy = "merge(d=" + std::to_string(depth) + " t=" + std::to_string(trials) + ")";  // avoid printing comma since escaping annoys
            merge_player::init(depth, trials);

            int order = depth * 10 + trials;
            int speed = order <= 15 ? 3 : (order <= 24 || order % 10 == 1 ? 2 : (order <= 33 ? 1 : 0));

            test_player(strategy, merge_player::player, GAMES[speed], GAMES[speed] <= MAX_THREADS, speed == 0);
        }
    }

    fout.close();
}

