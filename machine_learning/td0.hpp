#ifndef TD0_HPP
#define TD0_HPP

#include <fstream>
#include <sstream>
#include <vector>
#include "../game.hpp"

#ifdef WEBSITE
#include <emscripten/fetch.h>
#endif

// based on http://www.cs.put.poznan.pl/wjaskowski/pub/papers/Szubert2014_2048.pdf
// implements only the TD0 algorithm (Fig. 3 and Fig. 6)
class TD0: GameSimulator {
    static const inline std::string FILE_IDENTIFIER = "qp2048TD0";

    /*
    bit indexes of the board, for reference (top left is most significant):
    60 56 52 48
    44 40 36 32
    28 24 20 16
    12  8  4  0
    */

#ifdef TRAINING_ONLY
    // being able to set things as constexpr makes games run more than twice as fast, even if it makes things a bit ugly

    // tuple selection is from Fig. 3c of https://arxiv.org/pdf/1604.05085.pdf (later paper by same authors)
    static constexpr int N_TUPLE = 12;
    static constexpr int TUPLE_SIZE = 5;
    static constexpr int TUPLES[N_TUPLE * TUPLE_SIZE] = {  // flatten for speed
#else
    // defaults, can be changed if loading model from file
    int N_TUPLE = 12;
    int TUPLE_SIZE = 5;
    std::vector<int> TUPLES {
#endif
        0, 4, 8, 12, 16,
        16, 20, 24, 28, 32,
        0, 4, 8, 12, 20,
        16, 20, 24, 28, 36,
        0, 4, 8, 16, 20,
        16, 20, 24, 32, 36,
        0, 4, 8, 16, 32,
        16, 20, 24, 32, 48,
        0, 4, 8, 20, 24,
        16, 20, 24, 36, 40,
        4, 16, 20, 24, 36,
        20, 24, 28, 36, 52,
    };

    int TILE_CT, TUPLE_VALUES;
    float LEARNING_RATE;

    static constexpr float WINNING_EVAL = 1e8;
    
    float* lookup;  // lookup table for each tuple's score

    public:
    TD0(const int _tile_ct, const float _learning_rate):
        TILE_CT(_tile_ct),
        TUPLE_VALUES(N_TUPLE * ipow(_tile_ct, TUPLE_SIZE)),
        LEARNING_RATE(_learning_rate)
    {
        lookup = new float[TUPLE_VALUES]();  // page 5: " In all the experiments, the weights were initially set to 0"
#ifndef TRAINING_ONLY
        assert(TUPLES.size() == N_TUPLE * TUPLE_SIZE);
#endif
    }
#ifndef TRAINING_ONLY
    TD0(const int _n_tuple, const int _tuple_size, const std::vector<int>& _tuples, const int _tile_ct, const float _learning_rate):
        N_TUPLE(_n_tuple),
        TUPLE_SIZE(_tuple_size),
        TUPLES(_tuples.begin(), _tuples.end()),
        TILE_CT(_tile_ct),
        TUPLE_VALUES(N_TUPLE * ipow(_tile_ct, TUPLE_SIZE)),
        LEARNING_RATE(_learning_rate)
    {
        lookup = new float[TUPLE_VALUES]();
        assert(TUPLES.size() == N_TUPLE * TUPLE_SIZE);
    }
    TD0(const float _learning_rate, const std::string& filename): LEARNING_RATE(_learning_rate) {
        std::ifstream fin(filename, std::ios::binary);
        assert(fin.is_open());

        std::string identifier(FILE_IDENTIFIER.size(), '\0');
        fin.read(&identifier[0], FILE_IDENTIFIER.size());
        assert(identifier == FILE_IDENTIFIER);

        N_TUPLE = fin.get();
        TUPLE_SIZE = fin.get();
        TILE_CT = fin.get();

        TUPLE_VALUES = N_TUPLE * ipow(TILE_CT, TUPLE_SIZE);

        TUPLES = std::vector<int>(N_TUPLE * TUPLE_SIZE);
        for (int i = 0; i < N_TUPLE; ++i) {
            for (int j = 0; j < TUPLE_SIZE; ++j) {
                TUPLES[i * TUPLE_SIZE + j] = fin.get();
            }
        }

        std::string nonzero((TUPLE_VALUES + 7) / 8, '\0');
        fin.read(&nonzero[0], nonzero.size());

        lookup = new float[TUPLE_VALUES]();
        for (int i = 0; i < TUPLE_VALUES; ++i) {
            if ((nonzero[i >> 3] >> (i & 7)) & 1) fin.read(reinterpret_cast<char*>(&lookup[i]), sizeof(float));
        }
    }
#endif

    static TD0 best_model;
    static bool best_model_loaded;
    static void load_best();

    const std::string get_name() const {
        return "model_" + std::to_string(N_TUPLE) + "-" + std::to_string(TUPLE_SIZE) + "_" + std::to_string(TILE_CT) + "_" + std::to_string(LEARNING_RATE);
    }

    board_t test_model(int& fours) {
        const board_t tile_val0 = generate_random_tile_val();
        const board_t tile_val1 = generate_random_tile_val();
        fours += (tile_val0 == 2) + (tile_val1 == 2);
        board_t board = add_tile(add_tile(0, tile_val0), tile_val1);

        while (!game_over(board)) {// && get_max_tile(board) < TILE_CT - 1) {
            const board_t old_board = board;

            int attempts = 0x10000;
            while (old_board == board) {
                const int dir = pick_move(board);
                assert(0 <= dir && dir < 4);
                board = make_move(board, dir);

                if (game_over(board)) return board;

                assert(--attempts > 0);  // abort the game if it seems stuck
            }

            const board_t new_tile_val = generate_random_tile_val();
            if (new_tile_val == 2) ++fours;
            board = add_tile(board, new_tile_val);
        }

        return board;
    }

    // returns ending board from training game
    // TODO: record and return loss? is there a well-defined loss here?
    board_t train_model(int& fours) {
        const board_t tile_val0 = generate_random_tile_val();
        const board_t tile_val1 = generate_random_tile_val();
        fours += (tile_val0 == 2) + (tile_val1 == 2);
        board_t board = add_tile(add_tile(0, tile_val0), tile_val1);

        while (!game_over(board)) {// && get_max_tile(board) < TILE_CT - 1) {
            const int best_move = pick_move(board);
            const board_t after_board = make_move(board, best_move);
            const board_t rand_tile = generate_random_tile_val();
            const board_t new_board = add_tile(after_board, rand_tile);
            fours += rand_tile == 2;
            
            learn_evaluation(after_board, new_board);

            board = new_board;
        }

        return board;
    }
    void save(const std::string& filename) const {
        std::ofstream fout(filename, std::ios::binary);
        assert(fout.is_open());

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
            nonzero.back() |= (lookup[i] != 0) << (i & 7);
        }
        fout.write(nonzero.c_str(), nonzero.size());
        for (int i = 0; i < TUPLE_VALUES; ++i) {
            if (lookup[i] != 0) {
                fout.write(reinterpret_cast<char*>(&lookup[i]), sizeof(lookup[i]));
            }
        }
        fout.close();
    }
    float evaluate(const board_t board) const{
        // incentivize winning as soon as possible
        // # of fours is estimated by taking approximate # of moves and dividing by 10
        // better to underestimate # of 4's; that overestimates the score and causes a slightly larger penalty
        //if (get_max_tile(board) == TILE_CT - 1) return WINNING_EVAL - actual_score(board, 1015 / 10);
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
    const int pick_move(const board_t board) const {
        int best_move = -1;
        float best_score = 0;
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

    private:
    const int get_tuple(const int i, const board_t board) const {
        int tuple = 0;
        for (int j = 0; j < TUPLE_SIZE; ++j) {
            tuple *= TILE_CT;
            tuple += (board >> TUPLES[i * TUPLE_SIZE + j]) & 0xF;
        }
        return tuple;
    }
    const float _evaluate(const board_t board) const {
        float evaluation = 0;
        for (int i = 0; i < N_TUPLE; ++i) {
            evaluation += lookup[get_tuple(i, board)];
        }
        return evaluation;
    }
    const int calculate_reward(const board_t board, const board_t after_board) const {
        // difference of approximations works here since each board will have the same amount of fours spawn
        return approximate_score(after_board) - approximate_score(board);
    }
    void learn_evaluation(const board_t after_board, const board_t new_board) {
        if (game_over(new_board)) {
            // all future rewards will be 0, since the game has ended
            update_lookup(after_board, -evaluate(after_board));
            return;
        }
        const int best_next_move = pick_move(new_board);
        const board_t next_afterstate = make_move(new_board, best_next_move);
        const int next_reward = calculate_reward(new_board, next_afterstate);
        update_lookup(after_board, next_reward + evaluate(next_afterstate) - evaluate(after_board));
    }
    void update_lookup(const board_t after_board, float val) {
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
};


#ifdef WEBSITE
TD0 TD0::best_model = TD0(0, 0.0f);
bool TD0::best_model_loaded = false;
void TD0::load_best() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = [](emscripten_fetch_t* fetch) {
        std::cerr << "fetch succeeded!" << std::endl;
        std::istringstream is(std::string(fetch->data, fetch->numBytes));

        std::string identifier(FILE_IDENTIFIER.size(), '\0');
        is.read(&identifier[0], FILE_IDENTIFIER.size());
        assert(identifier == FILE_IDENTIFIER);

        const int n_tuple = is.get();
        const int tuple_size = is.get();
        const int tile_ct = is.get();

        const int tuple_values = n_tuple * ipow(tile_ct, tuple_size);

        std::vector<int> tuples(n_tuple * tuple_size);
        for (int i = 0; i < n_tuple; ++i) {
            for (int j = 0; j < tuple_size; ++j) {
                tuples[i * tuple_size + j] = is.get();
            }
        }

        TD0::best_model = TD0(n_tuple, tuple_size, tuples, tile_ct, 0);

        std::string nonzero((tuple_values + 7) / 8, '\0');
        is.read(&nonzero[0], nonzero.size());
        for (int i = 0; i < tuple_values; ++i) {
            if ((nonzero[i >> 3] >> (i & 7)) & 1) is.read(reinterpret_cast<char*>(&(TD0::best_model.lookup[i])), sizeof(float));
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
#elif defined LOAD_BEST_MODEL
TD0 TD0::best_model = TD0(0.0f, "machine_learning/model_8-6_16_0.000150/model_8-6_16_0.000150_1000.dat");
bool TD0::best_model_loaded = true;
void TD0::load_best() { /* do nothing, model already loaded */ }
#endif

#endif

