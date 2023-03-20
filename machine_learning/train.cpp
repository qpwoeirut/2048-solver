// at most one of these should be defined
#define TRAINING_ONLY
//#define TESTING

#include <filesystem>
#include <fstream>
#include <iostream>

#include "DoubleTD0.hpp"
#include "ExportedTD0.hpp"
#include "TD0.hpp"

constexpr double LEARNING_RATE = 0.001;
constexpr int EPOCHS = 2000;
constexpr int SAVE_INTERVAL = 100;
static_assert(EPOCHS % SAVE_INTERVAL == 0);

constexpr int TRAIN_GAMES = 10000;
constexpr int TEST_GAMES = 100000;
constexpr int MAX_GAMES = std::max(TRAIN_GAMES, TEST_GAMES);

constexpr int MIN_TILE = 3;   // getting 2^3 should always be guaranteed
constexpr int MAX_TILE = 15;

int results[MAX_TILE + 1];
int moves[MAX_GAMES];
int scores[MAX_GAMES];

#ifdef TESTING
std::unique_ptr<BaseModel> model(TD0::best_model);
#else
std::unique_ptr<BaseModel> model(new TD0(MAX_TILE + 1, LEARNING_RATE));
#endif

float calculate_average(const int arr[], const int n) {
    long long sum = 0;
    for (int i = 0; i < n; ++i) sum += arr[i];
    return float(sum) / n;
}

float calculate_median(const int arr[], const int n) {  // if n is even, returns mean of two middle elements
    std::vector<int> tmp(arr, arr + n);
    sort(tmp.begin(), tmp.end());
    return (n & 1) ? tmp[n / 2] : (tmp[(n - 1) / 2] + tmp[n / 2]) / 2.0;
}

void clear_results() {
    std::fill(results, results + MAX_TILE + 1, 0);
    std::fill(moves, moves + MAX_GAMES, 0);
    std::fill(scores, scores + MAX_GAMES, 0);
}

void print_results(const int games) {
    for (int i = MAX_TILE - 1; i >= MIN_TILE; --i) results[i] += results[i + 1];
    for (int i = MIN_TILE; i <= MAX_TILE; ++i) {
        std::cout << i << ' ' << results[i] << " (" << 100.0 * results[i] / games << ')' << std::endl;
    }
    std::cout << "Moves: " << calculate_average(moves, games) << ' ' << calculate_median(moves, games) << std::endl;
    std::cout << "Score: " << calculate_average(scores, games) << ' ' << calculate_median(scores, games) << std::endl;
}

void play_games(const bool train, const int games) {
    const long long start_time = get_current_time_ms();
    for (int i = 0; i < games; ++i) {
        int fours = 0;
        const board_t board = train ? model->train_model(fours) : model->test_model(fours);
        const int max_tile = get_max_tile(board);

        ++results[max_tile];

        moves[i] = count_moves_made(board, fours);
        scores[i] = actual_score(board, fours);
    }
    print_results(games);
    clear_results();
    const long long end_time = get_current_time_ms();
    const double time_taken = (end_time - start_time) / 1000.0;
    std::cout << "Time: " << time_taken << std::endl;
}

int main() {
    std::cout.precision(8);
    std::cout << "Learning rate = " << LEARNING_RATE << std::endl;

#ifndef TESTING
    std::filesystem::create_directory("machine_learning/" + model->get_name());
    for (int i = 1; i <= EPOCHS; ++i) {
        std::cout << "Epoch #" << i << " of " << EPOCHS << std::endl;
        play_games(true, TRAIN_GAMES);
        std::cout << std::endl;

        if (i % SAVE_INTERVAL == 0) {
            const std::string filename = "machine_learning/" + model->get_name() + "/" + model->get_name() + "_" + std::to_string(i) + ".dat";
            std::ofstream fout(filename, std::ios::binary);
            assert(fout.is_open());
            model->save(fout);
            fout.close();
        }
    }
#endif

//    std::cout << "Running " << TEST_GAMES << " testing games" << std::endl;
//    play_games(false, TEST_GAMES);
}

