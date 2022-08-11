#ifndef DOUBLE_TD0_HPP
#define DOUBLE_TD0_HPP

#include <algorithm>
#include "TD0.hpp"

class DoubleTD0 : public BaseModel {
    std::unique_ptr<TD0> abs_model, rel_model;

public:
    int USE_REL;  // relative model map tiles below this value to the same value
    DoubleTD0(std::unique_ptr<TD0> _abs_model, std::unique_ptr<TD0> _rel_model,
              const int _use_rel = 60) :
            BaseModel("qp2048DblTD0"), abs_model(std::move(_abs_model)),
            rel_model(std::move(_rel_model)), USE_REL(_use_rel) {
    }

#ifndef TRAINING_ONLY

    DoubleTD0(std::istream& is) : BaseModel("qp2048DblTD0") {
        std::string identifier(FILE_IDENTIFIER.size(), '\0');
        is.read(&identifier[0], FILE_IDENTIFIER.size());
        assert(identifier == FILE_IDENTIFIER);

        USE_REL = is.get();

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
        fout.put(static_cast<char>(USE_REL));
        abs_model->save(fout);
        fout.write("\0\0\0\0", 4);  // 4 nulls between the two models' data
        rel_model->save(fout);
    }

    const float evaluate(const board_t board) const override {
        return abs_evaluate(board) + rel_evaluate(board);
    }

    const float abs_evaluate(const board_t board) const {
        return abs_model->evaluate(cap_board(board));
    }

    const float rel_evaluate(const board_t board) const {
        return rel_model->evaluate(transform_relative(board));
    }

    void update_lookup(const board_t after_board, float val) override {
        abs_model->update_lookup(cap_board(after_board), val / 2);
        rel_model->update_lookup(transform_relative(after_board), val / 2);
    }

private:
    const int calculate_reward(const board_t board, const board_t after_board) const override {
        // difference of approximations works here since each board will have the same amount of fours spawn
        return approximate_score(after_board) - approximate_score(board);
    }

    const board_t cap_board(const board_t board) const {
        board_t capped_board = 0;
        for (int i = 0; i < 64; i += 4) {
            capped_board |= std::min((board >> i) & 0xF, static_cast<board_t>(abs_model->TILE_CT - 1)) << i;
        }
        return capped_board;
    }

    const board_t transform_relative(const board_t board) const {
        board_t new_board = 0;
        const int mx = get_max_tile(board);
        for (int i = 0; i < 64; i += 4) {
            const int val = (board >> i) & 0xF;
            if (val >= USE_REL) new_board |= std::min(mx - ((board >> i) & 0xF), static_cast<board_t>(rel_model->TILE_CT - 2)) << i;
            else if (val > 0) new_board |= static_cast<board_t>(rel_model->TILE_CT - 1) << i;
        }
        return new_board;
    }
};

#endif

