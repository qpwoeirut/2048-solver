#ifndef BASE_MODEL_HPP
#define BASE_MODEL_HPP

#include <string>
#include "../game.hpp"

class BaseModel: public GameSimulator {
    static constexpr float WINNING_EVAL = 1e8;

public:
    std::string FILE_IDENTIFIER;
    BaseModel(const std::string& file_id): FILE_IDENTIFIER(file_id) {}

    virtual const std::string get_name() const = 0;
    virtual const int pick_move(const board_t) const = 0;
    virtual void save(std::ostream&, const float) const = 0;
    virtual float evaluate(const board_t) const = 0;

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
private:
    virtual void learn_evaluation(const board_t, const board_t) = 0;
};

#endif
