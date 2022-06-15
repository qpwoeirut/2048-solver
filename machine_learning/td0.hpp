#ifndef TD0_HPP
#define TD0_HPP

#include <fstream>
#include "../game.hpp"

#ifdef WEBSITE
#include <emscripten/fetch.h>
#endif

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
        //{0, 4, 8, 12, 16, 32},
        //{16, 20, 24, 28, 32, 48},
        //{0, 4, 8, 12, 16, 28},
        //{16, 20, 24, 28, 32, 44},
    };

    const int tile_ct, tuple_values;
    const float learning_rate;

    static constexpr float WINNING_EVAL = 1e8;
    
    float* lookup;  // lookup table for each tuple's score

    public:
    TD0(const int _tile_ct, const float _learning_rate):
        tile_ct(_tile_ct),
        tuple_values(N_TUPLE * ipow(_tile_ct, TUPLE_SIZE)),
        learning_rate(_learning_rate)
    {
        lookup = new float[tuple_values]();  // page 5: " In all the experiments, the weights were initially set to 0"
    }
    TD0(const float _learning_rate): TD0(16, _learning_rate) {}
    TD0(const int _tile_ct, const float _learning_rate, const std::string& filename): TD0(_tile_ct, _learning_rate) {
        std::ifstream fin(filename);
        assert(fin.is_open());

        std::string line;
        int idx = 0;
        while (std::getline(fin, line)) {
            assert(fin.good());
            size_t eq_pos = line.find('=');
            if (eq_pos != std::string::npos) {
                idx = stoi(line.substr(0, eq_pos));
                line = line.substr(eq_pos + 1);
            }
            lookup[idx++] = stof(line);
        }
    }
    TD0(const float _learning_rate, const std::string& filename): TD0(16, _learning_rate, filename) {}

    static TD0 best_model;
    static bool best_model_loaded;
    static void load_best();

    void set_lookup(const int idx, const float new_val) {
        assert(idx < tuple_values);
        lookup[idx] = new_val;
    }

    const std::string get_name() const {
        return "model_" + std::to_string(N_TUPLE) + "-" + std::to_string(TUPLE_SIZE) + "_" + std::to_string(tile_ct) + "_" + std::to_string(learning_rate);
    }

    board_t test_model(int& fours) {
        const board_t tile_val0 = generate_random_tile_val();
        const board_t tile_val1 = generate_random_tile_val();
        fours += (tile_val0 == 2) + (tile_val1 == 2);
        board_t board = add_random_tile(add_random_tile(0, tile_val0), tile_val1);

        while (!game_over(board)) {// && get_max_tile(board) < tile_ct - 1) {
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

    // returns ending board from training game
    // TODO: record and return loss? is there a well-defined loss here?
    board_t train_model(int& fours) {
        const board_t tile_val0 = generate_random_tile_val();
        const board_t tile_val1 = generate_random_tile_val();
        fours += (tile_val0 == 2) + (tile_val1 == 2);
        board_t board = add_random_tile(add_random_tile(0, tile_val0), tile_val1);

        while (!game_over(board)) {// && get_max_tile(board) < tile_ct - 1) {
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
    void save(const std::string& filename) const {
        std::ofstream fout(filename);
        assert(fout.is_open());
        fout.precision(16);
        for (int i = 0; i < tuple_values; ++i) {
            if (lookup[i] != 0) {
                if (i == 0 || lookup[i-1] == 0) {
                    fout << i << '=';
                }
                fout << lookup[i] << '\n';
            }
        }
        fout.close();
    }
    float evaluate(const board_t board) const{
        // incentivize winning as soon as possible
        // # of fours is estimated by taking approximate # of moves and dividing by 10
        // better to underestimate # of 4's; that overestimates the score and causes a slightly larger penalty
        //if (get_max_tile(board) == tile_ct - 1) return WINNING_EVAL - actual_score(board, 1015 / 10);
        if (game_over(board)) return 0;
        const board_t flip_h_board = flip_h(board);
        const board_t flip_v_board = flip_v(board);
        const board_t flip_vh_board = flip_v(flip_h_board);
        return std::max(0.0f,  // all evaluations are assumed to be non-negative
                        _evaluate(board) + _evaluate(transpose(board)) +
                        _evaluate(flip_h_board) + _evaluate(transpose(flip_h_board)) +
                        _evaluate(flip_v_board) + _evaluate(transpose(flip_v_board)) +
                        _evaluate(flip_vh_board) + _evaluate(transpose(flip_vh_board)));
    }

    private:
    const std::array<int, N_TUPLE> get_tuples(const board_t board) const {
        std::array<int, N_TUPLE> tuples;
        for (int i = 0; i < N_TUPLE; ++i) {
            tuples[i] = 0;
            for (int j = 0; j < TUPLE_SIZE; ++j) {
                tuples[i] *= tile_ct;
                tuples[i] += (board >> TUPLES[i][j]) & 0xF;
            }
        }
        return tuples;
    }
    const float _evaluate(const board_t board) const {
        float evaluation = 0;
        for (const int tuple: get_tuples(board)) {
            evaluation += lookup[tuple];
        }
        return evaluation;
    }
    const int calculate_reward(const board_t board, const board_t after_board) const {
        // difference of approximations works here since each board will have the same amount of fours spawn
        return approximate_score(after_board) - approximate_score(board);
    }
    const int find_best_move(const board_t board) const {
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
        if (game_over(new_board)) {
            // all future rewards will be 0, since the game has ended
            update_lookup(after_board, -evaluate(after_board));
            return;
        }
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


#ifdef WEBSITE
TD0 TD0::best_model = TD0(0);
bool TD0::best_model_loaded = false;
void TD0::load_best() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = [](emscripten_fetch_t* fetch) {
        std::cerr << "fetch succeeded!" << std::endl;
        const std::string s(fetch->data, fetch->numBytes);
        std::string line;
        int idx = 0;
        for (int i = 0; i < fetch->numBytes; ++i) {
            if (fetch->data[i] == '=') {
                idx = stoi(line);
                line = "";
            } else if (fetch->data[i] == '\n') {
                TD0::best_model.set_lookup(idx, stof(line));
                ++idx;
                line = "";
            } else {
                line.push_back(fetch->data[i]);
            }
        }
        emscripten_fetch_close(fetch);

        TD0::best_model_loaded = true;
    };
    attr.onerror = [](emscripten_fetch_t* fetch) {
        std::cerr << "failed with status code " << fetch->status << std::endl;
        emscripten_fetch_close(fetch);
    };
    emscripten_fetch(&attr, "../model.dat");
}
#else
//TD0 TD0::best_model = TD0(0, "machine_learning/model_8-6_16_0.000250/model_8-6_16_0.000250_300.txt");
TD0 TD0::best_model = TD0(0);  // FIXME
bool TD0::best_model_loaded = true;
void TD0::load_best() { /* do nothing, model already loaded */ }
#endif

#endif

