# Notes

This stores some loose ideas that I might try in the future.

* Previous [ML work](https://arxiv.org/pdf/1604.05085.pdf)
* Compute a massive lookup table for some small subset of states
    * restrict to 2^10 and below?
    * assume that some amount of states are always empty
    * even with above conditions, this will still be at least 10^12 states, probably too big
* Restructure the solvers to have a strategy and a heuristic
* "Corner" and "snake" heuristics
* make the tester use fewer threads to avoid context switching overhead

