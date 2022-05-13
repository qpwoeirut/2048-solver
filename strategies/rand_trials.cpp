namespace rand_trials_strategy {
    /*
        Parameters:
            depth: depth to search, should be positive; note that search space increases exponentially with depth
            trials: trials for each move checked
    */

    int depth = 3;
    int trials = 5;
    heuristic_t evaluator = heuristics::score_heuristic;

    const long long MULT = 1e10;  // use integer math to speed up

    const long long helper(const board_t board, const int cur_depth) {
        if (cur_depth == 0) {
            return (evaluator(board) * MULT) << 2;  // move doesn't matter
        }

        long long best_score = 0;
        int best_move = 0;  // default best_move to 0; -1 causes issues with the packing in cases of full boards
        for (int i=0; i<4; ++i) {
            const board_t new_board = game::make_move(board, i);
            if (board == new_board || game::game_over(new_board)) continue;

            long long current_score = 0;
            for (int j=0; j<trials; ++j) {
                current_score += helper(game::add_random_tile(new_board, game::generate_random_tile_val()),
                                        cur_depth - 1) >> 2;  // extract score
            }
            if (best_score <= current_score) {
                best_score = current_score;
                best_move = i;
            }
        }
        return ((best_score / trials) << 2) | best_move;  // pack both score and move
    }
    const int player(const board_t board) {
        return helper(board, depth) & 3;
    }

    // this is ugly as heck but C++ doesn't let you pass capturing lambdas as function pointers so here we are
    // i tried classes too but those didn't work either
    void init(const int _depth, const int _trials, heuristic_t _evaluator) {
        depth = _depth;
        trials = _trials;
        evaluator = _evaluator;
    }
}

