#ifndef MONTE_CARLO_PLAYER_HPP
#define MONTE_CARLO_PLAYER_HPP

#include "Strategy.hpp"
#include "RandomPlayer.hpp"

class MonteCarloPlayer : public Strategy {
    /*
        Parameters:
            trials: random trials for each move
    */

    RandomPlayer random_player;

public:
    int trials;

    MonteCarloPlayer(const int _trials) {
        trials = _trials;
        random_player = RandomPlayer();
    }

    std::unique_ptr<Strategy> clone() override {
        return std::make_unique<MonteCarloPlayer>(trials);
    }

    const int pick_move(const board_t board) override {
        int best_score = 0;
        int best_move = -1;
        for (int i = 0; i < 4; ++i) {
            const board_t new_board = simulator.make_move(board, i);
            if (board == new_board) continue;

            int current_score = 0;
            for (int j = 0; j < trials; ++j) {
                current_score += run_trial(simulator.add_tile(new_board, simulator.generate_random_tile_val()));
            }
            if (best_score <= current_score) {
                best_score = current_score;
                best_move = i;
            }
        }
        return best_move;
    }

private:
    const int run_trial(board_t board) {
        while (!simulator.game_over(board)) {
            board = simulator.add_tile(simulator.make_move(board, random_player.pick_move(board)),
                                       simulator.generate_random_tile_val());
        }
        return heuristics::score_heuristic(board);
    }
};

#endif
