#ifndef TD0_HPP
#define TD0_HPP

#include <fstream>
#include <vector>
#include "BaseModel.hpp"

// based on http://www.cs.put.poznan.pl/wjaskowski/pub/papers/Szubert2014_2048.pdf
// implements only the TD0 algorithm (Fig. 3 and Fig. 6)
class TD0 : public BaseModel {
    /*
    bit indexes of the board, for reference (top left is most significant):
    60 56 52 48
    44 40 36 32
    28 24 20 16
    12  8  4  0
    */

#ifdef TRAINING_ONLY
    // being able to set things as constexpr makes games run more than twice as fast, even if it makes things a bit ugly
    static constexpr int N_TUPLE = 8;
    static constexpr int TUPLE_SIZE = 6;
    static constexpr int TUPLES[N_TUPLE * TUPLE_SIZE] = // flatten for speed
#else
    // defaults, can be changed if loading model from file
    int N_TUPLE = 12;
    int TUPLE_SIZE = 5;
    std::vector<int> TUPLES =
#endif
            {  // 8 tuples of size 6, from Fig. 3c of https://arxiv.org/pdf/1604.05085.pdf (later paper by same authors)
                    0,  4,  16, 20, 32, 48,
                    4,  8,  20, 24, 36, 52,
                    0,  4,  16, 20, 32, 36,
                    4,  8,  20, 24, 36, 40,
                    0,  4,  8,  12, 16, 32,
                    16, 20, 24, 28, 32, 48,
                    0,  4,  8,  12, 16, 28,
                    16, 20, 24, 28, 32, 44,
            };
//            {  // 12 tuples of size 5
//                    0, 4, 8, 12, 16,
//                    16, 20, 24, 28, 32,
//                    0, 4, 8, 12, 20,
//                    16, 20, 24, 28, 36,
//                    0, 4, 8, 16, 20,
//                    16, 20, 24, 32, 36,
//                    0, 4, 8, 16, 32,
//                    16, 20, 24, 32, 48,
//                    0, 4, 8, 20, 24,
//                    16, 20, 24, 36, 40,
//                    4, 16, 20, 24, 36,
//                    20, 24, 28, 36, 52,
//            };

    float LEARNING_RATE;
    int TUPLE_VALUES;

    float* lookup;  // lookup table for each tuple's score

public:
    int TILE_CT;
    TD0(const int _tile_ct, const float _learning_rate, const std::string& file_id = "qp2048TD0") :
            BaseModel(file_id), LEARNING_RATE(_learning_rate / N_TUPLE),
            TUPLE_VALUES(N_TUPLE * ipow(_tile_ct, TUPLE_SIZE)),
            TILE_CT(_tile_ct) {
        lookup = new float[TUPLE_VALUES]();  // page 5: " In all the experiments, the weights were initially set to 0"
#ifndef TRAINING_ONLY
        assert(TUPLES.size() == N_TUPLE * TUPLE_SIZE);
#endif
    }

#ifndef TRAINING_ONLY
    TD0(const int _n_tuple, const int _tuple_size, const std::vector<int>& _tuples, const int _tile_ct,
        const float _learning_rate, const std::string& file_id = "qp2048TD0") :
            BaseModel(file_id), LEARNING_RATE(_learning_rate, / _n_tuple),
            N_TUPLE(_n_tuple),
            TUPLE_SIZE(_tuple_size),
            TUPLES(_tuples.begin(), _tuples.end()),
            TUPLE_VALUES(N_TUPLE * ipow(_tile_ct, TUPLE_SIZE)),
            TILE_CT(_tile_ct) {
        lookup = new float[TUPLE_VALUES]();
        assert(TUPLES.size() == N_TUPLE * TUPLE_SIZE);
    }

    TD0(const float _learning_rate, std::istream& is, const std::string& file_id = "qp2048TD0") :
            BaseModel(file_id), LEARNING_RATE(_learning_rate) {
        std::string identifier(FILE_IDENTIFIER.size(), '\0');
        is.read(&identifier[0], FILE_IDENTIFIER.size());
        assert(identifier == FILE_IDENTIFIER);

        N_TUPLE = is.get();
        TUPLE_SIZE = is.get();
        TILE_CT = is.get();

        TUPLE_VALUES = N_TUPLE * ipow(TILE_CT, TUPLE_SIZE);

        TUPLES = std::vector<int>(N_TUPLE * TUPLE_SIZE);
        for (int i = 0; i < N_TUPLE; ++i) {
            for (int j = 0; j < TUPLE_SIZE; ++j) {
                TUPLES[i * TUPLE_SIZE + j] = is.get();
            }
        }

        std::string nonzero((TUPLE_VALUES + 7) / 8, '\0');
        is.read(&nonzero[0], nonzero.size());

        lookup = new float[TUPLE_VALUES]();
        for (int i = 0; i < TUPLE_VALUES; ++i) {
            if ((nonzero[i >> 3] >> (i & 7)) & 1) is.read(reinterpret_cast<char*>(&lookup[i]), sizeof(float));
        }

        LEARNING_RATE /= N_TUPLE;
    }

#endif

    const std::string get_name() const override {
        return "model_" + std::to_string(N_TUPLE) + "-" + std::to_string(TUPLE_SIZE) + "_" + std::to_string(TILE_CT) +
               "_" + std::to_string(LEARNING_RATE);
    }

    void save(std::ostream& fout) const override {
        fout.write(FILE_IDENTIFIER.c_str(), FILE_IDENTIFIER.size());

        fout.put(static_cast<char>(N_TUPLE));
        fout.put(static_cast<char>(TUPLE_SIZE));
        fout.put(static_cast<char>(TILE_CT));

        for (int i = 0; i < N_TUPLE * TUPLE_SIZE; ++i) {
            fout.put(static_cast<char>(TUPLES[i]));
        }

        std::string nonzero;
        for (int i = 0; i < TUPLE_VALUES; ++i) {
            if ((i & 7) == 0) nonzero.push_back(static_cast<char>(0));
            nonzero.back() |= (lookup[i] == 0) << (i & 7);
        }
        fout.write(nonzero.c_str(), nonzero.size());
        for (int i = 0; i < TUPLE_VALUES; ++i) {
            if (lookup[i] != 0) {
                fout.write(reinterpret_cast<char*>(&lookup[i]), sizeof(lookup[i]));
            }
        }
    }

    const float evaluate(const board_t board) const override {
        // incentivize winning as soon as possible
        // # of fours is estimated by taking approximate # of moves and dividing by 10
        // better to underestimate # of 4's; that overestimates the score and causes a slightly larger penalty
        //if (get_max_tile(board) == TILE_CT - 1) return WINNING_EVAL - actual_score(board, 1015 / 10);

        if (game_over(board)) return 0;
        const board_t flip_h_board = flip_h(board);
        const board_t flip_v_board = flip_v(board);
        const board_t flip_vh_board = flip_v(flip_h_board);
        return std::max(0.0f,  // all evaluations are assumed to be non-negative
                        eval_single(board) + eval_single(transpose(board)) +
                        eval_single(flip_h_board) + eval_single(transpose(flip_h_board)) +
                        eval_single(flip_v_board) + eval_single(transpose(flip_v_board)) +
                        eval_single(flip_vh_board) + eval_single(transpose(flip_vh_board)));
    }

    void update_lookup(const board_t after_board, float val) override {
        val *= LEARNING_RATE;

        const board_t tafter_board  = transpose(after_board);
        const board_t flip_h_board  = flip_h(after_board);  const board_t tflip_h_board  = transpose(flip_h_board);
        const board_t flip_v_board  = flip_v(after_board);  const board_t tflip_v_board  = transpose(flip_v_board);
        const board_t flip_vh_board = flip_v(flip_h_board); const board_t tflip_vh_board = transpose(flip_vh_board);
        for (int i = 0; i < N_TUPLE; ++i) {
            lookup[get_tuple(i, after_board)] += val;
            lookup[get_tuple(i, tafter_board)] += val;

            lookup[get_tuple(i, flip_h_board)] += val;
            lookup[get_tuple(i, tflip_h_board)] += val;

            lookup[get_tuple(i, flip_v_board)] += val;
            lookup[get_tuple(i, tflip_v_board)] += val;

            lookup[get_tuple(i, flip_vh_board)] += val;
            lookup[get_tuple(i, tflip_vh_board)] += val;
        }
    }

    static TD0 best_model;
    static bool best_model_loaded;

    static void load_best();

private:
    const int get_tuple(const int i, const board_t board) const {
        int tuple = 0;
        for (int j = 0; j < TUPLE_SIZE; ++j) {
            tuple *= TILE_CT;
            tuple += (board >> TUPLES[i * TUPLE_SIZE + j]) & 0xF;
        }
        return tuple;
    }

    const float eval_single(const board_t board) const {
        float evaluation = 0;
        for (int i = 0; i < N_TUPLE; ++i) {
            evaluation += lookup[get_tuple(i, board)];
        }
        return evaluation;
    }

    const int calculate_reward(const board_t board, const board_t after_board) const override {
        // difference of approximations works here since each board will have the same amount of fours spawn
        return approximate_score(after_board) - approximate_score(board);
    }
};

#endif
