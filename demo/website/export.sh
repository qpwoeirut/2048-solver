em++ --std=c++20 export_players.cpp -o players.js \
--no-entry -sWASM_BIGINT \
-O3 \
-Wall \
-sEXPORTED_FUNCTIONS=_init_game,_random_player,_spam_corner_player,_ordered_player,_rotating_player,_rand_trials_player,_init_rand_trials_strategy,_minimax_player,_init_minimax_strategy,_expectimax_player,_init_expectimax_strategy,_monte_carlo_player,_init_monte_carlo_strategy