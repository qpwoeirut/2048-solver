# 2048 solvers

There are multiple strategies that are implemented.


## Random
This solver simply picks a random valid move.


## Corner
This solver randomly picks between left and up;
If both moves aren't allowed, it picks randomly between right and down.

## Ordered
This solver tries the same move order each time.
If left is allowed, it moves left.
Otherwise if up is allowed, it moves up.
Otherwise if right is allowed, it moves right.
Otherwise it moves down.

