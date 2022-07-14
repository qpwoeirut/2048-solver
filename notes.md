# Notes

This stores some loose ideas that I might try in the future.

* Previous [ML work](https://arxiv.org/pdf/1604.05085.pdf)
  * This is far more complicated than anything I'll be able to do myself, given my lack of experience
    * Started by implementing this [paper](https://www.cs.put.poznan.pl/wjaskowski/pub/papers/Szubert2014_2048.pdf) from the same authors
  * [Other](https://arxiv.org/pdf/2111.11090.pdf) [work](https://arxiv.org/pdf/1606.07374.pdf)
* Compute a massive lookup table for some small subset of states
  * restrict to 2^10 and below?
  * assume that some amount of states are always empty
  * even with above conditions, this will still be at least 10^12 states, probably too big
* "snake" heuristic
* some sort of "distance" heuristic between tiles of adjacent values
* penalty heuristic for distance between tiles of same values (didn't really work)
* try converting corner/wall building heuristics to be difference between adjacent tiles multiplied by a weight
  * this is essentially a monotonicity heuristic i think?
* use concept of "afterstates" to cut down the depth searched by one
* for wall heuristics, increase depth if the ordering has an "inversion"
* use geo mean instead of arith mean for expectimax?
  * issues with overflow; GMP? seems rather slow
* write a tester for difficult board positions to tune heuristics
* figure out some import system (CMake?)
* make 3 cheater AIs: one that knows tile placements, one that controls tile placements and obviously cheats, one that controls tile placements but pretends that it doesn't
* optimize cache time/memory more
  * is storing transposed/rotated states or searching for them in cache worth it? will need to benchmark. also might vary based on strategy
* investigate using better variant of alpha-beta pruning with search order heuristic
* use compile-time lookup tables for heuristics?
* `player.simulator.play(player, fours)` is rather ugly; clean up somehow?
* at some point unit tests should exist
* also before running the full statistic-generation tests I should run a mini-test with a small amount of games to make sure everything works properly and won't take forever

## Website
* add descriptions of each option
* display the current search depth? and % completion of search?
* make depth customizable
* allow user to create their own multiplication weight heuristic
