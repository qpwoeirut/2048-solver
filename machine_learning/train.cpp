#include <iostream>
#include "td0.hpp"

TD0 model(0.001);
constexpr int TRAINING_GAMES = 100000;

void play_game(const int game_idx) {
    int fours = 0;
    const board_t board = model.train(fours);
    const int max_tile = get_max_tile(board);

    const int moves = count_moves_made(board, fours);
    const int score = actual_score(board, fours);

    if (game_idx % 100 == 0) std::cout << game_idx << ' ' << max_tile << ' ' << moves << ' ' << score << std::endl;
}

int main() {
    for (int i = 0; i < TRAINING_GAMES; ++i) {
        play_game(i);
    }
}
