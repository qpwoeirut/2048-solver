clang++ -std=c++20 -O3 -Wall -pthread -funroll-loops -flto -march=native -fconstexpr-steps=0x600000 tester.cpp -o tester.out
echo "Compiled tester.cpp!"
./tester.out
rm tester.out
