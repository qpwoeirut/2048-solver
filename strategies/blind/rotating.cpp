namespace rotating_player {

    int current_move = 0;

    const int player(const board_t board) {
        while (board == game::make_move(board, current_move)) current_move = (current_move + 1) % 4;
        return current_move++;  // increment so that next move will start with different value
    }
}
