namespace corner_player {
    std::mt19937 move_gen(get_current_time_ms());
    std::uniform_int_distribution<> move_distrib(0, 1);
    int player(const board_t board) {
        int move = move_distrib(move_gen);
        if (board != game::make_move(board, move)) return move;
        if (board != game::make_move(board, move ^ 1)) return move ^ 1;

        if (board != game::make_move(board, move + 2)) return move + 2;
        return (move ^ 1) + 2;
    }

}
