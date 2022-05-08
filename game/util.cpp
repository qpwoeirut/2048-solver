using board_t = uint64_t;
using row_t = uint16_t;

long long get_current_time_ms() {
    const std::chrono::time_point now = std::chrono::system_clock::now();
    const long long seconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return seconds;
}

std::mt19937 move_gen(get_current_time_ms());
std::uniform_int_distribution move_distrib(0, 3);

int random_move() {
    return move_distrib(move_gen);
}

void print_board(const board_t board) {
    for (int r=48; r>=0; r-=16) {
        for (int c=12; c>=0; c-=4) {
            std::cout << ((board >> (r + c)) & 0xF) << ' ';
        }
        std::cout << '\n';
    }
}

int get_max_tile(const board_t board) {
    int max_tile = 0;
    for (int i=0; i<64; i+=4) max_tile = std::max(max_tile, (int)((board >> i) & 0xF));
    return max_tile;
}

