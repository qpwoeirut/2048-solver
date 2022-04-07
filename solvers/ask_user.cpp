namespace user_player {
    inline int move_to_int(const char c) {
        if (c == 'L') return 0;
        if (c == 'U') return 1;
        if (c == 'R') return 2;
        if (c == 'D') return 3;
        return -1;
    }

    int player(const board_t board) {
        print_board(board);
        std::cout << "Next move? (L, U, R, D)\n";

        char move;
        std::cin >> move;

        int dir = move_to_int(move);
        if (dir == -1) {
            std::cout << "Invalid move!\n";
            return player(board);
        }
        if (board == game::make_move(board, dir)) {
            std::cout << "No change!\n";
            return player(board);
        }
        return dir;
    }
}

