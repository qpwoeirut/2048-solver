#include <pybind11/pybind11.h>

PYBIND11_MODULE(players, m) {
    m.doc() = "Solving strategies for 2048 written in C++ and exported to Python";

    m.def("subtract", [](int i, int j) { return i - j; }, "testing");
}
