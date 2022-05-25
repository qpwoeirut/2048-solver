#ifndef SPAM_CORNER_PLAYER_HPP
#define SPAM_CORNER_PLAYER_HPP

#include "Strategy.hpp"

class SpamCornerPlayer: public Strategy {
    public:
    const int player(const board_t board) {
        const int move = random_move() & 1;
        if (board != game::make_move(board, move)) return move;
        if (board != game::make_move(board, move ^ 1)) return move ^ 1;

        if (board != game::make_move(board, move + 2)) return move + 2;
        return (move ^ 1) + 2;
    }
};

#endif

