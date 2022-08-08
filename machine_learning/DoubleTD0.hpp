#ifndef DOUBLE_TD0_HPP
#define DOUBLE_TD0_HPP

#include "TD0.hpp"

class DoubleTD0 : public BaseModel {
    std::unique_ptr<TD0> abs_model, rel_model;

public:
    const int USE_REL;
    DoubleTD0(const float _learning_rate, std::unique_ptr<TD0> _abs_model, std::unique_ptr<TD0> _rel_model,
              const int _use_rel = 4) :
            BaseModel("qp2048DblTD0", _learning_rate), abs_model(std::move(_abs_model)),
            rel_model(std::move(_rel_model)), USE_REL(_use_rel) {
        abs_model->LEARNING_RATE = LEARNING_RATE;
        rel_model->LEARNING_RATE = LEARNING_RATE;
    }

#ifndef TRAINING_ONLY

    DoubleTD0(const float _learning_rate, std::istream& is, const int _use_rel = 4) :
            BaseModel("qp2048DblTD0", _learning_rate), USE_REL(_use_rel) {
        std::string identifier(FILE_IDENTIFIER.size(), '\0');
        is.read(&identifier[0], FILE_IDENTIFIER.size());
        assert(identifier == FILE_IDENTIFIER);

        abs_model = std::make_unique<TD0>(_learning_rate, is);

        for (int i = 0; i < 4; i++) assert(is.get() == 0);  // 4 nulls between the two models' data

        rel_model = std::make_unique<TD0>(_learning_rate, is);
    }

#endif  // TRAINING_ONLY

    const std::string get_name() const override {
        return "abs" + abs_model->get_name() + "__rel" + rel_model->get_name();
    }

    void save(std::ostream& fout) const override {
        fout.write(FILE_IDENTIFIER.c_str(), FILE_IDENTIFIER.size());
        abs_model->save(fout);
        fout.write(std::string(4, '\0'));  // 4 nulls between the two models' data
        rel_model->save(fout);
    }

    const float evaluate(const board_t board) const override {
        return abs_model->evaluate(board) + rel_evaluate(board);
    }

    const float rel_evaluate(const board_t board) const {
        const int max_tile = get_max_tile(board);
        return max_tile >= rel_model->TILE_CT + USE_REL ?
               rel_model->evaluate(transform_board_relative(board, max_tile)) :
               0.0f;
    }

    void learn_evaluation(const board_t after_board, const board_t new_board) override {
        abs_model->learn_evaluation(after_board, new_board);
        const int after_max_tile = get_max_tile(after_board);
        if (after_max_tile >= rel_model->TILE_CT + USE_REL) {
            const int new_max_tile = get_max_tile(new_board);
            rel_model->learn_evaluation(
                    transform_board_relative(after_board, after_max_tile),
                    transform_board_relative(new_board, new_max_tile));
        }
    }

private:
    const int calculate_reward(const board_t board, const board_t after_board) const override {
        // difference of approximations works here since each board will have the same amount of fours spawn
        return approximate_score(after_board) - approximate_score(board);
    }

    const int transform_board_relative(const board_t board, const int value) const {
        board_t new_board = 0;
        for (int i = 0; i < 64; i += 4) {
            new_board |= (value - ((board >> i) & 0xF)) << i;
        }
        return new_board;
    }
}

#endif

