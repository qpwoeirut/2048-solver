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
* add heuristic penalty for dying?
* for wall heuristics, increase depth if the ordering has an "inversion"
