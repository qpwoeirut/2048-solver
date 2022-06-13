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
    static constexpr int TUPLE_VALUES = N_TUPLE * (1 << 24);  // 4 * 16^6
    
    float lookup[TUPLE_VALUES];  // lookup table for each tuple's score

    const float learning_rate;
    public:
    TD0(const float _learning_rate): learning_rate(_learning_rate) {
        std::fill(lookup, lookup + TUPLE_VALUES, 0);  // page 5: " In all the experiments, the weights were initially set to 0"
    }
    // TODO: save and read lookup table from a file

    // returns ending board from training game
    // TODO: record and return loss instead
    board_t train(int& fours) {
        const board_t tile_val0 = generate_random_tile_val();
        const board_t tile_val1 = generate_random_tile_val();
        fours += (tile_val0 == 2) + (tile_val1 == 2);
        board_t board = add_random_tile(add_random_tile(0, tile_val0), tile_val1);

        while (!game_over(board)) {
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
    float evaluate(const board_t board) {
        const board_t flip_h_board = flip_h(board);
        const board_t flip_v_board = flip_v(board);
        const board_t flip_vh_board = flip_v(flip_h_board);
        return _evaluate(board) + _evaluate(transpose(board)) +
               _evaluate(flip_h_board) + _evaluate(transpose(flip_h_board)) +
               _evaluate(flip_v_board) + _evaluate(transpose(flip_v_board)) +
               _evaluate(flip_vh_board) + _evaluate(transpose(flip_vh_board));
    }

    private:
    std::array<int, N_TUPLE> get_tuples(const board_t board) {
        std::array<int, N_TUPLE> tuples;
        for (int i = 0; i < N_TUPLE; ++i) {
            for (int j = 0; j < TUPLE_SIZE; ++j) {
                tuples[i] <<= 4;
                tuples[i] |= (board >> TUPLES[i][j]) & 0xF;
            }
        }
        return tuples;
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

