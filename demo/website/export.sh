em++ --std=c++20 -O3 -Wall -funroll-loops -fconstexpr-steps=0x500000 export_players.cpp -o players.js \
-I /usr/local/Cellar/google-sparsehash/2.0.4/include/ \
--no-entry -lembind -sWASM_BIGINT -sALLOW_MEMORY_GROWTH -sENVIRONMENT=web -sFILESYSTEM=0 -flto \
#-g -sASSERTIONS=2 -sDEMANGLE_SUPPORT=1 -sSAFE_HEAP=1 -sSTACK_OVERFLOW_CHECK=2
