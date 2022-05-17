g++ -std=c++20 -O3 -Wall -pthread -funroll-loops -DUSE_CACHE benchmark.cpp -o benchmark.out
echo "Compiled benchmark.cpp!"
./benchmark.out
rm benchmark.out
