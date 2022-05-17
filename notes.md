# Notes

This stores some loose ideas that I might try in the future.

* Previous [ML work](https://arxiv.org/pdf/1604.05085.pdf)
* Compute a massive lookup table for some small subset of states
    * restrict to 2^10 and below?
    * assume that some amount of states are always empty
    * even with above conditions, this will still be at least 10^12 states, probably too big
* "wall" and "snake" heuristics
* make the tester use fewer threads to avoid context switching overhead
* use concept of "afterstates" to cut down the depth searched by one
* for minimax/expectimax, don't eval states with very low probability
* use alpha-beta pruning for minimax
* for deterministic strategies, cache calculated states
  * for cache of board evaluations, use gp_hash_table or cc_hash_table
* change heuristic penalty for dying?
* for wall heuristics, increase depth if the ordering has an "inversion"
* use geo mean instead of arith mean for expectimax?
  * issues with overflow; GMP? seems rather slow
* increase depth if # of distinct tiles is high
* write a strict full wall builder heuristic which penalizes inversions
* tiebreak wall builder with score or merge
* write a tester for difficult board positions to tune heuristics
* deal with parallel accesses/writes when testing minimax/expectimax solvers in parallel
  * or just ignore it? it'll slow things down a little but nothing will be *wrong*
* figure out some import system
  * hardcoding the path to sparsehash in export.sh nonideal
* clear up cache parts that aren't needed
* look into [emscripten optimizations](https://emscripten.org/docs/optimizing/Optimizing-Code.html)