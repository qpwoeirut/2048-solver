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

// assumes that only 2's had spawned, which is a good enough approximation
// creating a tile of 2^n adds 2^n to the score, and requires two 2^(n-1) tiles
// creating each of those added 2^(n-1) to the score, and following the recursive pattern gets n * 2^n
// technically we want (n-1) * 2^n since the 2's spawning don't add to the score
int approximate_score(const board_t board) {
    int score = 0;
    for (int i=0; i<64; i+=4) {
        const uint8_t tile = (board >> i) & 0xF;
        score += tile <= 1 ? 0 : (tile - 1) * (1 << tile);
    }
    return score;
}

int count_empty(uint16_t n) {
    // a somewhat understandable way to count set bits
    // https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
    int empty_ct;
    for (empty_ct = 0; n > 0; ++empty_ct) {
        n &= n - 1;
    }
    return empty_ct;
}

