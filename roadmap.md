# 2048 Solver Roadmap

This project is intended to "solve" the popular [2048 tile-merging game](https://github.com/gabrielecirulli/2048).
My overall goal is to create an ML model that can reach 2048 at least 15% of the time.
Since I have literally never touched ML before, 15% success is probably an extremely ambitious goal.

Although I've done some research on prior work and how successful previous projects have been, I want to write all the solver code on my own.
My 2048 game implementation, of course, references the [original game](https://github.com/gabrielecirulli/2048) in order to replicate its mechanics.

For the purposes of this roadmap, a "success" means reaching the 2048 tile, unless another tile is specified.


# Part 0: Game Implementation
In order to test my strategies, I needed to implement the game.
This implementation needed to be as fast as possible, since many of the strategies would be simulating games in memory as they played.
I used the original [2048](https://github.com/gabrielecirulli/2048) repo to ensure that my game worked in the same way.

I wrote all the code except for some [bitwise magic](https://github.com/nneonneo/2048-ai/blob/master/2048.cpp#L38-L48) that transposes the board myself.
The [README](/game/README.md) for the `game` directory has more information.


# Part 1: "Traditional" solver
Once the game was implemented, I started by writing a traditional algorithmic solver.
I originally aimed to have a "traditional" solver that can succeed 80% of the time which I can hopefully use to train an ML model.

This part has been split into multiple stages.
Each stage adds new strategies and heuristics to test.


## Stage 1

The best strategy from this stage succeeded 98.8% of the time.
I ran at least 500 test games for each player using an AWS EC2 Linux c6g.large instance.
See this [CSV file](/results.csv) for all the results.

### Naive Ideas
I began with simple strategies: a random player and several players that spam moves in some order.
None of these players do any calculations before picking moves.
The current board state is irrelevant to them, except when making sure a move is legal.

These players almost always died before reaching the 512 tile.

### Search with random trials
This is a general strategy which aims to maximize a heuristic function at a certain depth `d` by running `t` trials.
It's similar to a brute force search, but only `t` of the possible tile placements by the computer are simulated.
The move with the highest average evaluation is selected.

I wrote a heuristic that approximates the score of a given board position (`score`) and another that counts the number of empty tiles (`merge`).
Neither are particularly sophisticated; the arrangement of the tiles on the board is not taken into account.

The `score` heuristic was slightly more successful than the `merge` heuristic.
At the highest depth/trial parameters (`d=5` and `t=5`), the `score` heuristic reached 2048 81.2% of the time and 4096 with an 11.2% success rate.
`merge`'s success rate was 70.6% for 2048 and 6.4% for 4096.

### Monte Carlo tree search
This idea is taken from this [project](https://github.com/ronzil/2048-AI).

This approach is appealing because of its simplicity.
To make a move, the solver tries every possible move by simulating it and then simulating further random moves until the game ends.
Each move has some amount of simulations `t`, and the scores from those games are averaged to find the score for that move.
The move which results in the highest score is chosen for the actual game.

This strategy was the most successful out of all the Stage 1 players.
Not only did it run much faster, but it reached 2048 98.8% of the time and got to 4096 with a 56.6% success rate with `t=2000`.
Some tests with a lower parameter of `t` were just as, if not slightly more, successful at reaching 4096, so it's possible that increasing `t` starts to yield diminishing returns.


## Stage 2

### Heuristics
I added a `corner` heuristic which tries to keep large tiles near the corner of the board.
It gives each position a weight to multiply it by, and the evaluation is the sum of the tiles multiplied by the respective weights.
The weight grid is:
```
10 5  2  1
5  3  1  0
2  1  0  0
1  0  0  0
```
The final evaluation is the maximum across all four corners, which allows the player to switch the corner it's building on if that's optimal.

### Minimax search
This is from [here](https://github.com/ovolve/2048-AI).

This stage only has the brute force version of minimax implemented.
There's no pruning or caching; every single possible state is evaluated.
This makes the player very slow, so the maximum depth I tested was `d=4`, where the player searches up to 4 moves ahead.

From what I know, one of the downsides of using minimax is that the computer's tile placement is random and not necessarily adversarial.
As a result, using minimax leads to a very cautious player.


### Expectimax Optimization
This strategy is used by the best two traditional solvers I found ([here](https://github.com/nneonneo/2048-ai) and [here](https://github.com/MaartenBaert/2048-ai-emscripten)), which completely blow all the other traditional solvers out of the water.

Expectimax is similar to minimax, but instead of assuming that the computer will place at the worst possible position, the expected value based on the probabilities of the computer's choice is maximized.
This stage's implementation of expectimax also doesn't implement any pruning or caching.


## Stage 3
I added a simple [benchmark](/benchmark.cpp) program to provide some direction in speeding up the game implementation.
I also added `-funroll-loops` to the compilation flags, which sped things up.

I've decided that from Stage 3 onwards, I'll focus more on the strategies that are very successful.
The "blind" strategies won't be run in the final tests and the less effective heuristics (`score` and `merge`) will be dropped as well.
Hopefully this will make testing run a little faster (and lower my AWS bill).

### Website
While waiting for the testing of Stage 2 to finish, I put together a [website demo](https://qpwoeirut.github.io/2048/).

### Heuristics
I added two heuristics designed to make the AI build tiles against the wall.

### Depth Picker
Both the minimax and expectimax strategies take a long time to evaluate boards with lots of empty tiles, but those positions are usually not very important.
I added a depth picker, which decreases the depth to search for boards with few tiles and increases the depth on boards that are nearly full.

### Alpha-Beta Pruning
Work in progress

### Evaluation Caching
Work in progress


### Other Ideas
I was discussing possible 2048 strategies with a friend, who pointed out the possibility that many positions are "useless," in the sense that the move picked from that position won't heavily affect the outcome of the game.
If it's possible to identify the "usefulness" of a position, the useless positions can be solved by simply making a random move.
This strategy should speed up games, especially in the early stages, allowing for more testing and/or training.


# Part 2: ML-based solver
This part will probably be even harder.
There's lots of prior work for non-ML solvers but not as much on ML-based solvers.

Although I have some knowledge of the basic theory behind ML, I have yet to implement an actual project utilizing it.
My first thought is to feed the model the current board state and then have it pick a move.
Then to train the model, hundreds or thousands of games can be run, and the average score from these games can be used as feedback.
I suspect it won't actually be that simple though, otherwise somebody else would have already implemented it.

The model could also simply provide a board evaluation (which I think is similar to how Stockfish works).
Then it can be used as the heuristic for a minimax or expectimax solver.

I'll have to do more research on ML strategies when the time comes.


## Links to Prior Work
These are only the solvers that provided the success rates up-front or seemed good enough that I wanted to write them down.
There are many more repos out there which didn't list success rates.
* [macroxue's solver](https://github.com/macroxue/2048-ai) uses some strategy that I don't understand and reaches 32768 with a 74.0% success rate and 65536 with a 3.0% rate. It seems to use a large lookup table for difficult positions, combined with expectimax
* [aszczepanski's solver](https://github.com/aszczepanski/2048) uses an n-tuple network with expectimax and reaches 32768 with a 69% success rate when making one move per second
* [nneonneo's solver](https://github.com/nneonneo/2048-ai) uses expectimax and reaches 16384 with a 94% success rate
* [MaartenBaert's solver](https://github.com/MaartenBaert/2048-ai-emscripten) uses expectimax and seems to be very good, although I didn't poke around enough to find tile success stats. Its README claims that it's slightly better than the above solver
* [kcwu's solver](https://github.com/kcwu/2048-c) uses expectimax with a helper minimax search to avoid dying and reaches the 8192 tile with a 96% success rate
* [ronzil's solver](https://github.com/ronzil/2048-AI) uses Monte Carlo tree search and reaches 2048 80% of the time when using 100 simulations per move and reaches 4096 70% of the time when using 10000 runs. It also has a [C++ implementation](https://github.com/ronzil/2048-ai-cpp)
* [ovolve's solver](https://github.com/ovolve/2048-AI) uses minimax and reaches 2048 with a success rate of "about 90%"
* [acrantel's solver](https://github.com/acrantel/2048) uses expectimax and reaches 2048 with a success rate of "approximately 80%"
* [datumbox's solver](https://github.com/datumbox/Game-2048-AI-Solver) uses minimax and can reach 2048 "about 70-80%" of the time with a depth of 7
* [vpn1997's solver](https://github.com/vpn1997/2048-Ai) uses expectimax and reaches 2048 with a 60% success rate
* [xtrp's solver](https://github.com/xtrp/jupiter) uses Monte Carlo tree search and reaches 2048 with ~60% success rate when using 50 simulations per move
* [navjindervirdee's solver](https://github.com/navjindervirdee/2048-deep-reinforcement-learning) uses deep reinforcement learning and reaches the 2048 tile 10% of the time

