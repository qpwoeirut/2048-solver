#ifndef STRATEGY_HPP
#define STRATEGY_HPP

#include "../util.hpp"

class Strategy {
    public:
    virtual ~Strategy() = default;
    virtual std::unique_ptr<Strategy> clone() = 0;
    virtual const int pick_move(const board_t board) = 0;
    virtual void reset() {}  // most strategies won't require any resetting
};

#endif

