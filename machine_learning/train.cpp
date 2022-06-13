#include <iostream>
#include "td0.hpp"


constexpr int EPOCHS = 100;

constexpr int MAX_GAMES = 20000;
constexpr int TRAIN_GAMES = MAX_GAMES;
constexpr int TEST_GAMES = 1000;

constexpr int MIN_TILE = 3;   // getting 2^3 should always be guaranteed
constexpr int MAX_TILE = 11;  // for now, the model will stop playing once 2048 is reached

int results[MAX_TILE + 1];
int moves[MAX_GAMES];
int scores[MAX_GAMES];

TD0 model(0.001);

float calculate_average(const int arr[], const int n) {
    long long sum = 0;
    for (int i = 0; i < n; ++i) sum += arr[i];
    return float(sum) / n;
}
float calculate_median(const int arr[], const int n) {  // if n is even, returns mean of two middle elements
    std::vector<int> tmp(arr, arr+n);
    sort(tmp.begin(), tmp.end());
    return (n & 1) ? tmp[n / 2] : (tmp[(n-1) / 2] + tmp[n / 2]) / 2.0;
}

void clear_results() {
    std::fill(results, results + MAX_TILE + 1, 0);
    std::fill(moves, moves + MAX_GAMES, 0);
    std::fill(scores, scores + MAX_GAMES, 0);
}

void print_results(const int games) {
    for (int i = MAX_TILE - 1; i >= MIN_TILE; --i) {
        results[i] += results[i+1];
    }
    for (int i= MIN_TILE; i <= MAX_TILE; ++i) {
        std::cout << i << ' ' << results[i] << " (" << 100.0 * results[i] / games << ')' << std::endl;
    }
    std::cout << "Moves: " << calculate_average(moves, games) << ' ' << calculate_median(moves, games) << std::endl;
    std::cout << "Score: " << calculate_average(scores, games) << ' ' << calculate_median(scores, games) << std::endl;
}

void play_training_games() {
    for (int i = 0; i < TRAIN_GAMES; ++i) {
        int fours = 0;
        const board_t board = model.train_model(fours);
        const int max_tile = get_max_tile(board);
        
        ++results[max_tile];

        moves[i] = count_moves_made(board, fours);
        scores[i] = actual_score(board, fours);
    }
    print_results(TRAIN_GAMES);
    clear_results();
}

void play_testing_games() {
    for (int i = 0; i < TEST_GAMES; ++i) {
        int fours = 0;
        const board_t board = model.test_model(fours);
        const int max_tile = get_max_tile(board);
        
        ++results[max_tile];

        moves[i] = count_moves_made(board, fours);
        scores[i] = actual_score(board, fours);
    }
    print_results(TEST_GAMES);
    clear_results();
}

int main() {
    for (int i = 0; i < EPOCHS; ++i) {
        std::cout << "Epoch #" << i+1 << " of " << EPOCHS << std::endl;
        play_training_games();
        std::cout << std::endl;
    }

    std::cout << "Running " << TRAIN_GAMES << " training games" << std::endl;
    play_testing_games();
}
