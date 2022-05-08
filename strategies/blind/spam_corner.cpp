namespace corner_player {
    const int player(const board_t board) {
        const int move = random_move() & 1;
        if (board != game::make_move(board, move)) return move;
        if (board != game::make_move(board, move ^ 1)) return move ^ 1;

        if (board != game::make_move(board, move + 2)) return move + 2;
        return (move ^ 1) + 2;
    }

}
