# Notes

This stores some loose ideas that I might try in the future.

* Previous [ML work](https://arxiv.org/pdf/1604.05085.pdf)
* Compute a massive lookup table for some small subset of states
    * restrict to 2^10 and below?
    * assume that some amount of states are always empty
    * even with above conditions, this will still be at least 10^12 states, probably too big
* "wall" and "snake" heuristics
  * some sort of "distance" heuristic between tiles of adjacent values
* make the tester use fewer threads to avoid context switching overhead
* use concept of "afterstates" to cut down the depth searched by one
* use alpha-beta pruning for minimax
* for wall heuristics, increase depth if the ordering has an "inversion"
* use geo mean instead of arith mean for expectimax?
  * issues with overflow; GMP? seems rather slow
* write a strict full wall builder heuristic which penalizes inversions
* write a tester for difficult board positions to tune heuristics
* deal with parallel accesses/writes when testing minimax/expectimax solvers in parallel
  * or just ignore it? it'll slow things down a little but nothing will be *wrong*
* figure out some import system
  * hardcoding the path to sparsehash in export.sh nonideal
* i think the actual 2048 game starts with 2 tiles on the board?
  * once this is resolved, move count can be calculated using sum of tiles and # of fours generated
* make 3 cheater AIs: one that knows tile placements, one that controls tile placements and obviously cheats, one that controls tile placements but pretends that it doesn't              
* optimize cache time/memory more

## Website
* persist AI settings with localstorage
* change website's warning from 8s to 10s, make the warning at start actually take 10s
* add descriptions of each option
  * add note that the pause time is only a minimum
* display the current search depth? and % completion of search?
* make depth customizable
* allow user to create their own multiplication weight heuristic
* hide heuristics section if selected strategy doesn't use heuristic
* remove you win screen?
* make mobile view look less awful
* add a board editor?
* add replay of games?
