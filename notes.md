# Notes

This stores some loose ideas that I might try in the future.

* Previous [ML work](https://arxiv.org/pdf/1604.05085.pdf)
* Compute a massive lookup table for some small subset of states
    * restrict to 2^10 and below?
    * assume that some amount of states are always empty
    * even with above conditions, this will still be at least 10^12 states, probably too big
* "snake" heuristic
* some sort of "distance" heuristic between tiles of adjacent values
* make the tester use fewer threads to avoid context switching overhead
* use concept of "afterstates" to cut down the depth searched by one
* for wall heuristics, increase depth if the ordering has an "inversion"
* use geo mean instead of arith mean for expectimax?
  * issues with overflow; GMP? seems rather slow
* write a strict full wall builder heuristic which penalizes inversions
* write a tester for difficult board positions to tune heuristics
* deal with parallel accesses/writes when testing expectimax strategy in parallel
  * or just ignore it? it'll slow things down a little but nothing will be *wrong*
* figure out some import system
  * hardcoding the path to sparsehash in export.sh nonideal
* make 3 cheater AIs: one that knows tile placements, one that controls tile placements and obviously cheats, one that controls tile placements but pretends that it doesn't              
* optimize cache time/memory more
  * is storing transposed/rotated states or searching for them in cache worth it? will need to benchmark. also might vary based on strategy
* investigate using better variant of alpha-beta pruning with search order heuristic
* at some point the website and selenium demos should be fixed

## Website
* persist AI settings with localstorage
* add descriptions of each option
* display the current search depth? and % completion of search?
* make depth customizable
* allow user to create their own multiplication weight heuristic
* remove you win screen?
* add a board editor?
* add replay of games?
