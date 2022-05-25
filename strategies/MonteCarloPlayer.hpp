#ifndef MONTE_CARLO_PLAYER_HPP
#define MONTE_CARLO_PLAYER_HPP

#include "Strategy.hpp"
#include "RandomPlayer.hpp"

class MonteCarloPlayer: public Strategy {
    /*
        Parameters:
            trials: random trials for each move
    */

    static RandomPlayer random_player;

    public:
    int trials;
    MonteCarloPlayer(const int _trials) {
        trials = _trials;
    }

    private:
    const int run_trial(board_t board) {
        while (!game::game_over(board)) {
            board = game::add_random_tile(game::make_move(board, random_player.pick_move(board)),
                                          game::generate_random_tile_val());
        }
        return heuristics::score_heuristic(board);
    }

    const int pick_move(const board_t board) override {
        int best_score = 0;
        int best_move = -1;
        for (int i=0; i<4; ++i) {
            const board_t new_board = game::make_move(board, i);
            if (board == new_board) continue;

            int current_score = 0;
            for (int j=0; j<trials; ++j) {
                current_score += run_trial(game::add_random_tile(new_board, game::generate_random_tile_val()));
            }
            if (best_score <= current_score) {
                best_score = current_score;
                best_move = i;
            }
        }
        return best_move;
    }
};

#endif

