cd ..
clang++ -std=c++20 -O3 -Wall -funroll-loops -flto -mcpu=native -fconstexpr-steps=0x600000 machine_learning/train.cpp -o machine_learning/train.out
echo "Compiled train.cpp!"
./machine_learning/train.out
rm machine_learning/train.out
