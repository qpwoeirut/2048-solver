#ifndef STRATEGY_HPP
#define STRATEGY_HPP

#include "../util.hpp"

class Strategy {
    public:
    virtual const int pick_move(const board_t board) = 0;  // pure virtual, requires override
};

#endif
