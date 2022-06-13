clang++ -std=c++20 -O3 -Wall -funroll-loops -flto -mcpu=native -fconstexpr-steps=0x600000 train.cpp -o train.out
echo "Compiled train.cpp!"
./train.out
rm train.out
