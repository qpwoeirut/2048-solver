clang++ -std=c++20 -O3 -Wall -pthread -funroll-loops -flto -mcpu=native -fconstexpr-steps=0x500000 benchmark.cpp -o benchmark.out \
-DREQUIRE_DETERMINISTIC -DTESTING

echo "Compiled benchmark.cpp!"
./benchmark.out
rm benchmark.out

# top -stats time,command,cpu,mem,instrs,cycles | grep benchmark
