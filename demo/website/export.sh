em++ --std=c++20 -O3 -DWEBSITE -Wall -funroll-loops -fconstexpr-steps=0x500000 export_players.cpp -o players.js \
--no-entry -lembind -sWASM_BIGINT -sALLOW_MEMORY_GROWTH -sENVIRONMENT=web -sFILESYSTEM=0 -flto \
-sFETCH \
-I /usr/local/include
# -g -sASSERTIONS=2 -sDEMANGLE_SUPPORT=1 -sSAFE_HEAP=1 -sSTACK_OVERFLOW_CHECK=2
