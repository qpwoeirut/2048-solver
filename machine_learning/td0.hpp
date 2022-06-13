#ifndef TD0_HPP
#define TD0_HPP

#include "../game.hpp"

// based on http://www.cs.put.poznan.pl/wjaskowski/pub/papers/Szubert2014_2048.pdf
// implements only the TD0 algorithm (Fig. 3 and Fig. 6)
class TD0: GameSimulator {
    // tuple selection is from Fig. 3c of https://arxiv.org/pdf/1604.05085.pdf (later paper by same authors)
    static constexpr int N_TUPLE = 4;
    static constexpr int TUPLE_SIZE = 6;
    /*
    bit indexes of the board, for reference (top left is most significant):
    60 56 52 48
    44 40 36 32
    28 24 20 16
    12  8  4  0
    */
    static constexpr int TUPLES[N_TUPLE][TUPLE_SIZE] = {
        {0, 4, 16, 20, 32, 48},
        {4, 8, 20, 24, 36, 52},
        {0, 4, 16, 20, 32, 36},
        {4, 8, 20, 24, 36, 40},
    };

    // stop once 2048 is reached. as higher tiles are reached, the model seems to lose accuracy in the earlier stages
    static constexpr int TILE_CT = 12;
    static constexpr int TUPLE_VALUES = N_TUPLE * 2985984; // 4 * 12^6

    static constexpr float WINNING_EVAL = 1e5;
    
    float lookup[TUPLE_VALUES];  // lookup table for each tuple's score

    const float learning_rate;
    public:
    TD0(const float _learning_rate): learning_rate(_learning_rate) {
        std::fill(lookup, lookup + TUPLE_VALUES, 0);  // page 5: " In all the experiments, the weights were initially set to 0"
    }
    board_t test_model(int& fours) {
        const board_t tile_val0 = generate_random_tile_val();
        const board_t tile_val1 = generate_random_tile_val();
        fours += (tile_val0 == 2) + (tile_val1 == 2);
        board_t board = add_random_tile(add_random_tile(0, tile_val0), tile_val1);

        while (!game_over(board) && get_max_tile(board) < TILE_CT - 1) {
            const board_t old_board = board;

            int attempts = 0x10000;
            while (old_board == board) {
                const int dir = find_best_move(board);
                assert(0 <= dir && dir < 4);
                board = make_move(board, dir);

                if (game_over(board)) return board;

                assert(--attempts > 0);  // abort the game if it seems stuck
            }

            const board_t new_tile_val = generate_random_tile_val();
            if (new_tile_val == 2) ++fours;
            board = add_random_tile(board, new_tile_val);
        }

        return board;
    }

    // TODO: save and read lookup table from a file

    // returns ending board from training game
    // TODO: record and return loss? is there a well-defined loss here?
    board_t train_model(int& fours) {
        const board_t tile_val0 = generate_random_tile_val();
        const board_t tile_val1 = generate_random_tile_val();
        fours += (tile_val0 == 2) + (tile_val1 == 2);
        board_t board = add_random_tile(add_random_tile(0, tile_val0), tile_val1);

        while (!game_over(board) && get_max_tile(board) < TILE_CT - 1) {
            const int best_move = find_best_move(board);
            const board_t after_board = make_move(board, best_move);
            const board_t rand_tile = generate_random_tile_val();
            const board_t new_board = add_random_tile(after_board, rand_tile);
            fours += rand_tile == 2;
            
            learn_evaluation(after_board, new_board);

            board = new_board;
        }

        return board;
    }

    private:
    std::array<int, N_TUPLE> get_tuples(const board_t board) {
        std::array<int, N_TUPLE> tuples;
        for (int i = 0; i < N_TUPLE; ++i) {
            for (int j = 0; j < TUPLE_SIZE; ++j) {
                tuples[i] *= TILE_CT;
                tuples[i] += (board >> TUPLES[i][j]) & 0xF;
            }
        }
        return tuples;
    }
    float evaluate(const board_t board) {
        if (get_max_tile(board) == TILE_CT - 1) return WINNING_EVAL - approximate_score(board);  // incentivize winning as soon as possible
        const board_t flip_h_board = flip_h(board);
        const board_t flip_v_board = flip_v(board);
        const board_t flip_vh_board = flip_v(flip_h_board);
        return _evaluate(board) + _evaluate(transpose(board)) +
               _evaluate(flip_h_board) + _evaluate(transpose(flip_h_board)) +
               _evaluate(flip_v_board) + _evaluate(transpose(flip_v_board)) +
               _evaluate(flip_vh_board) + _evaluate(transpose(flip_vh_board));
    }
    float _evaluate(const board_t board) {
        float evaluation = 0;
        for (const int tuple: get_tuples(board)) {
            evaluation += lookup[tuple];
        }
        return evaluation;
    }
    int calculate_reward(const board_t board, const board_t after_board) {
    // difference of approximations works here since each board will have the same amount of fours spawn
        return approximate_score(after_board) - approximate_score(board);
    }
    int find_best_move(const board_t board) {
        int best_move = -1;
        float best_score = -1e9;
        for (int i = 0; i < 4; ++i) {
            const board_t after_board = make_move(board, i);
            if (board == after_board) continue;

            const int reward = calculate_reward(board, after_board);
            const float eval = reward + evaluate(after_board);
            if (best_score <= eval) {
                best_score = eval;
                best_move = i;
            }
        }
        assert(best_move != -1);

        return best_move;
    }
    void learn_evaluation(const board_t after_board, const board_t new_board) {
        if (game_over(new_board)) return;
        const int best_next_move = find_best_move(new_board);
        const board_t next_afterstate = make_move(new_board, best_next_move);
        const int next_reward = calculate_reward(new_board, next_afterstate);
        update_lookup(after_board, next_reward + evaluate(next_afterstate) - evaluate(after_board));
    }
    void update_lookup(const board_t after_board, const float val) {
        const board_t flip_h_board = flip_h(after_board);
        const board_t flip_v_board = flip_v(after_board);
        const board_t flip_vh_board = flip_v(flip_h_board);
        _update_lookup(after_board, val);   _update_lookup(transpose(after_board), val);
        _update_lookup(flip_h_board, val);  _update_lookup(transpose(flip_h_board), val);
        _update_lookup(flip_v_board, val);  _update_lookup(transpose(flip_v_board), val);
        _update_lookup(flip_vh_board, val); _update_lookup(transpose(flip_vh_board), val);
    }
    void _update_lookup(const board_t after_board, const float val) {
        for (const int tuple: get_tuples(after_board)) {
            lookup[tuple] += learning_rate * val;
        }
    }
};

#endif

