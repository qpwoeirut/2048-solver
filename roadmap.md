# 2048 Solver Roadmap

This project is intended to "solve" the popular [2048 tile-merging game](https://github.com/gabrielecirulli/2048).
My overall goal is to create a ML model that can reach 2048 at least 15% of the time.
Since I have literally never touched ML before, 15% success is probably an extremely ambitious goal.

Although I've done some research on prior work and how successful previous projects have been, I want to write all the solver code on my own.
My 2048 game implementation, of course, will be referencing the [original game](https://github.com/gabrielecirulli/2048) in order to replicate its mechanics.

For the purposes of this roadmap, a "success" means reaching the 2048 tile, unless another tile is specified.


## Roadmap: Part 1
I want to start by writing a traditional algorithmic solver.
I aim to have a "traditional" solver that can succeed 80% of the time which I can hopefully use to train a ML model.
There are many possible strategies for the first solver.
I may end up implementing several of them.


### Monte Carlo tree search
This idea is taken from this [project](https://github.com/ronzil/2048-AI).

This approach is appealing because of its simplicity.
To make a move, the solver tries every possible move by simulating it and then simulating further random moves.
Each move has some amount of simulations, whose scores are averaged to find the score for that move.
The move which results in the highest score is chosen for the actual game.


### Minimax search with alpha-beta pruning
This is from [here](https://github.com/ovolve/2048-AI).

I will do more research on this technique soon, since I currently haven't taken the time to properly understand it.
From what I know, one of the downsides of using minimax is that the computer's tile placement is random and not necessarily adversarial.
As a result, using minimax leads to a very cautious player.
I suppose if one is aiming for high success rate that is fine, but it seems nonoptimal when attempting to achieve the highest possible score.


### Expectimax Optimization
This strategy is used by the best two solvers I found ([here](https://github.com/nneonneo/2048-ai) and [here](https://github.com/MaartenBaert/2048-ai-emscripten)), which completely blow all the other traditional solvers out of the water.

Expectimax seems similar to minimax, but instead of assuming that the computer will place at the worst possible position, the expected value based on the probabilities of the computer's choice is maximized.
Again, I'll need to do some more research to fully understand the strategy.

Besides the switch from minimax to expectimax, these solvers also use a very efficient board representation and seems to have more efficient heuristics.


## Links to Prior Work for non-ML solvers
These are only the solvers that provided the success rates up-front.
There are many more repos out there which didn't list success rates.
* [nneonneo's solver](https://github.com/nneonneo/2048-ai) uses expectimax and reaches 16384 with a 94% success rate
* [MaartenBaert's solver](https://github.com/MaartenBaert/2048-ai-emscripten) uses expectimax and seems to be very good, although I didn't poke around enough to find tile success stats. Its README claims that it's slightly better than the above solver
* [kcwu's solver](https://github.com/kcwu/2048-c) uses expectimax with a helper minimax search to avoid dying and reaches the 8192 tile with a 96% success rate
* [ronzil's solver](https://github.com/ronzil/2048-AI) uses Monte Carlo tree search and reaches 2048 80% of the time when using 100 simulations per move and reaches 4096 70% of the time when using 10000 runs. It also has a [C++ implementation](https://github.com/ronzil/2048-ai-cpp)
* [ovolve's solver](https://github.com/ovolve/2048-AI) uses minimax and reaches 2048 with a success rate of "about 90%"
* [acrantel's solver](https://github.com/acrantel/2048) uses expectimax and reaches 2048 with a success rate of "approximately 80%"
* [datumbox's solver](https://github.com/datumbox/Game-2048-AI-Solver) uses minimax and can reach 2048 "about 70-80%" of the time with a depth of 7
* [vpn1997's solver](https://github.com/vpn1997/2048-Ai) uses expectimax and reaches 2048 with a 60% success rate
* [xtrp's solver](https://github.com/xtrp/jupiter) uses Monte Carlo tree search and reaches 2048 with ~60% success rate when using 50 simulations per move


