g++ -std=c++20 -O3 -Wall -pthread -funroll-loops -fconstexpr-steps=0x500000 -DREQUIRE_DETERMINISTIC benchmark.cpp -o benchmark.out
echo "Compiled benchmark.cpp!"
./benchmark.out
rm benchmark.out

# top -stats time,command,cpu,mem,instrs,cycles | grep benchmark
