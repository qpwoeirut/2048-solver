g++ -std=c++20 -O3 -Wall -pthread -funroll-loops -fconstexpr-steps=0x500000 tester.cpp -o tester.out
echo "Compiled tester.cpp!"
./tester.out
rm tester.out
