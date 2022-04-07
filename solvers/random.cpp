#include <iostream>

#include "../game/game.cpp"

std::mt19937 move_gen(8);  // seeding go brrr
std::uniform_int_distribution<> move_distrib(0, 3);

int random_solver(const board_t board) {
    int move;
    do {  // this *shouldn't* infinite loop but i guess we will see...
        move = move_distrib(move_gen);
    }
    while (board == make_move(board, move));
    return move;
}

int main() {
    init();
    for (int game=0; game<1000; ++game) {
        const board_t final_board = play_game(random_solver);
        std::cout << "max tile: " << get_max_tile(final_board) << std::endl;
    }
}

