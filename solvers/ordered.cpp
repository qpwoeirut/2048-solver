namespace ordered_player {
    const int player(const board_t board) {
        if (board != game::make_move(board, 0)) return 0;
        if (board != game::make_move(board, 1)) return 1;
        if (board != game::make_move(board, 2)) return 2;
        return 3;
    }
}

