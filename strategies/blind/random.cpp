namespace random_strategy {
    const int player(const board_t board) {
        int move;
        do {  // this *shouldn't* infinite loop but i guess we will see...
            move = random_move();
        }
        while (board == game::make_move(board, move));
        return move;
    }
}

