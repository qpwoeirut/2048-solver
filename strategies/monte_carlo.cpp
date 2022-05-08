namespace monte_carlo_player {
    /*
        Parameters:
            trials: random trials for each move
    */

    int trials = 100;
    const int (*secondary_player)(const board_t) = random_player::player;

    const int run_trial(board_t board) {
        while (!game::game_over(board)) {
            board = game::add_random_tile(game::make_move(board, secondary_player(board)));
        }
        return heuristics::score_heuristic(board);
    }

    const int player(const board_t board) {
        int best_score = 0;
        int best_move = -1;
        for (int i=0; i<4; ++i) {
            const board_t new_board = game::make_move(board, i);
            if (board == new_board) continue;

            int current_score = 0;
            for (int j=0; j<trials; ++j) {
                current_score += run_trial(game::add_random_tile(new_board));
            }
            if (best_score <= current_score) {
                best_score = current_score;
                best_move = i;
            }
        }
        return best_move;
    }

    // this is ugly as heck but C++ doesn't let you pass capturing lambdas as function pointers so here we are
    // i tried classes too but those didn't work either
    void init(const int _trials) {  
        trials = _trials;
    }
}

