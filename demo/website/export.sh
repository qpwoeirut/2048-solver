em++ --std=c++20 -O3 -Wall -funroll-loops export_players.cpp -o players.js -DUSE_CACHE \
-I /usr/local/Cellar/google-sparsehash/2.0.4/include/ \
--no-entry -sWASM_BIGINT -sALLOW_MEMORY_GROWTH -sENVIRONMENT=web -sFILESYSTEM=0 \
-sEXPORTED_FUNCTIONS=_init_game,_random_player,_spam_corner_player,_ordered_player,_rotating_player,_rand_trials_player,_init_rand_trials_strategy,_minimax_player,_init_minimax_strategy,_expectimax_player,_init_expectimax_strategy,_monte_carlo_player,_init_monte_carlo_strategy
#-g -sASSERTIONS=2 -sDEMANGLE_SUPPORT=1 -sSAFE_HEAP=1 -sSTACK_OVERFLOW_CHECK=2
