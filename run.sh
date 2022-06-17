clang++ -std=c++20 -O3 -Wall -pthread -funroll-loops -flto -mcpu=native -fconstexpr-steps=0x600000 tester.cpp -o tester.out \
-DLOAD_BEST_MODEL

echo "Compiled tester.cpp!"
./tester.out
rm tester.out
