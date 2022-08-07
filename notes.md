# Notes

This stores some loose ideas that I might try in the future.

* Previous [ML work](https://arxiv.org/pdf/1604.05085.pdf)
  * This is far more complicated than anything I'll be able to do myself, given my lack of experience
    * Started by implementing this [paper](https://www.cs.put.poznan.pl/wjaskowski/pub/papers/Szubert2014_2048.pdf) from the same authors
  * [Other](https://arxiv.org/pdf/2111.11090.pdf) [work](https://arxiv.org/pdf/1606.07374.pdf)
* For ML, instead of encoding tile values, encode tile exponents' difference with largest exponent
  * Store differences from 0-7 and 8+, and have another network that stores exponents from 0-7 and 8+
  * Allows for tuples of size 6, 9^6 is smaller than current tuple size (15^5) and 14^6
* Compute a massive lookup table for some small subset of states
  * restrict to 2^10 and below?
  * assume that some amount of states are always empty
  * even with above conditions, this will still be at least 10^12 states, probably too big
* "snake" heuristic
* some sort of "distance" heuristic between tiles of adjacent values
* penalty heuristic for distance between tiles of same values (didn't really work)
* instead of monotonicity for an entire row/column, just have penalties for "trapped" tiles on the edge
* use concept of "afterstates" to cut down the depth searched by one
* for wall heuristics, increase depth if the ordering has an "inversion"
* write a tester for difficult board positions to tune heuristics
* figure out some import system (CMake?, [check examples](https://github.com/ttroy50/cmake-examples/blob/master/01-basic/H-third-party-library/CMakeLists.txt))
* make 3 cheater AIs: one that knows tile placements, one that controls tile placements and obviously cheats, one that controls tile placements but pretends that it doesn't
* optimize cache time/memory more
  * is storing transposed/rotated states or searching for them in cache worth it? will need to benchmark. also might vary based on strategy
* investigate using better variant of alpha-beta pruning with search order heuristic
* `player.simulator.play(player, fours)` is rather ugly; clean up somehow?
* at some point unit tests should exist
* also before running the full statistic-generation tests I should run a mini-test with a small amount of games to make sure everything works properly and won't take forever
* investigate more compiler flags for optimization
  * clang and emscripten: Ofast + ffast-math + fno-rtti + fno-exceptions
  * [emscripten](https://github.com/emscripten-core/emscripten/blob/main/src/settings.js):
    * GL_EMULATE_GLES_VERSION_STRING_FORMAT=0 + GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=0
    * INCOMING_MODULE_JS_API
    * STRICT?
    * BINARYEN_IGNORE_IMPLICIT_TRAPS=1
    * LEGALIZE_JS_FFI=0?
    * SUPPORT_ERRNO=0
    * MINIMAL_RUNTIME=1?
    * MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION=1? or MINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION=1
    * USES_DYNAMIC_ALLOC=0
    * SUPPORT_LONGJMP=0
    * HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS=0
    * ALLOW_UNIMPLEMENTED_SYSCALLS=0?
* investigate using worker-specific emscripten flags (BUILD_AS_WORKER, PROXY_TO_WORKER) or modularization (MODULARIZE, EXPORT_ES6)
* investigate using emscripten's file packager for the `model.dat` (now `model.bmp`) file
  * also investigate using indexeddb for fetching or turn FETCH_SUPPORT_INDEXEDDB off 
* Parallelize searches within a single game

## Website
* display the current search depth? and % completion of search?
* make depth customizable
* allow user to create their own multiplication weight heuristic

## Probably infeasible/unhelpful
* use geo mean instead of arith mean for expectimax?
  * issues with overflow; GMP? seems rather slow
* try converting corner/wall building heuristics to be difference between adjacent tiles multiplied by a weight
  * this is essentially a monotonicity heuristic i think?