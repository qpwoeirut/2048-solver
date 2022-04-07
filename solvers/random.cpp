namespace random_player {
    std::mt19937 move_gen(8);  // seeding go brrr
    std::uniform_int_distribution<> move_distrib(0, 3);

    int player(const board_t board) {
        int move;
        do {  // this *shouldn't* infinite loop but i guess we will see...
            move = move_distrib(move_gen);
        }
        while (board == game::make_move(board, move));
        return move;
    }
}

