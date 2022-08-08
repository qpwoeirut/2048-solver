#ifndef STRATEGY_HPP
#define STRATEGY_HPP

#include <random>
#include "../game.hpp"
#include "../util.hpp"

#ifdef REQUIRE_DETERMINISTIC
std::mt19937 seed_generator{8};  // this will generate each strategy's seeds
#else
std::mt19937 seed_generator{static_cast<unsigned int>(get_current_time_ms())};  // this will generate each strategy's seeds
#endif

class Strategy {
public:
    GameSimulator simulator{static_cast<long long>(seed_generator())};

    virtual ~Strategy() = default;

    virtual std::unique_ptr<Strategy> clone() = 0;

    virtual const int pick_move(const board_t board) = 0;

    virtual void reset() {}  // most strategies won't require any resetting
};


#endif
