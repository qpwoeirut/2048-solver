# 2048 solvers

So far all the strategies implemented are very basic.
I've categorized each into "Naive" and "Simple".


## Naive

### Random
This solver simply picks a random valid move.


### Corner
This strategy randomly picks between left and up.
If both moves aren't allowed, it picks randomly between right and down.


### Ordered
This solver tries the same move order each time.
If left is allowed, it moves left.
Otherwise if up is allowed, it moves up.
Otherwise if right is allowed, it moves right.
Otherwise it moves down.


## Simple

### Merge
This strategy attempts to merge as many tiles as possible at a certain search depth.
It's essentially a recursive brute force.
Each move checked runs some amount of trials to simulate the randomly placed tiles.

The success rate of this solver is surprisingly high; setting `depth` to 4 and `trials` to 4 can get the 2048 tile with a 53% success rate (out of 100 games).
Each game, however, takes around 3.2 seconds to complete.

Here's the abridged output of the run that those numbers are based on:
```
Testing merge strategy...
...
Playing 100 games took 317.996 seconds.
...
8 100 (100)
9 99 (99)
10 97 (97)
11 53 (53)
12 0 (0)
```

Using `depth=3` and `trials=6` is faster but only succeeds 12% of the time.
```
Testing merge strategy...
...
Playing 100 games took 119.103 seconds.
...
8 100 (100)
9 98 (98)
10 80 (80)
11 12 (12)
12 0 (0)
```
