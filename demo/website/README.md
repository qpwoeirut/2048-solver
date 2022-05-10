# Website Demo

This [repo](https://github.com/qpwoeirut/2048) holds the code for the live website demo of the strategies.

The C++ code is compiled into WASM using [emscripten](https://emscripten.org/).

The `WASM_BIGINT` flag is used so that the 64-bit integers work in the JS code, where the board is treated as a BigInt.