namespace rotating_player {

    int current_move = 0;

    const int player(const board_t board) {
        do {
            current_move = (current_move + 1) % 4;
        } while (board == game::make_move(board, current_move));
        return current_move;
    }
}
