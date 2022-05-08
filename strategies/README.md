# 2048 solvers

See the [results](/results) directory for the performances of each strategy.


## Blind
These strategies pick moves without precalculating future outcomes.

### Random
This strategy picks a random valid move.


### Spam Corner
This strategy randomly picks between left and up.
If both moves aren't allowed, it picks randomly between right and down.


### Ordered
This solver tries the same move order each time.
If left is allowed, it moves left.
Otherwise if up is allowed, it moves up.
Otherwise if right is allowed, it moves right.
Otherwise it moves down.


### Rotating
This solver plays the same moves in order.
It starts with left, then plays up, then plays right, then plays down, and repeats.


## Simple

### Merge
This strategy attempts to merge as many tiles as possible at a certain search depth.
It's essentially a recursive brute force.
Each move checked runs some amount of trials to simulate the randomly placed tiles.

The `d` (depth) and `t` (trials) parameters can be changed, although the time to run grows exponentially in `d`, so usually `d <= 4`.
The number of trials doesn't seem to have much of an effect beyond around 5 or 6.


### Monte Carlo tree search
For each possible move, this solver runs `t` random games from the resulting board and averages the score from all those games.
The move that has the best average score is picked.

The time it takes for this solver is approximately linear in `t`.
The rate of improvement starts to decrease as `t` gets to around a thousand.

