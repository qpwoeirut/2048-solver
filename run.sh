g++ -std=c++20 -O3 -Wall -pthread -funroll-loops -DUSE_CACHE tester.cpp -o tester.out
echo "Compiled tester.cpp!"
./tester.out
rm tester.out
