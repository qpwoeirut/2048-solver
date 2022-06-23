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
[game.md](/game.md) has more information.


# Part 1: "Traditional" solver
Once the game was implemented, I started by writing a traditional algorithmic solver.
I originally aimed to have a "traditional" solver that can succeed 80% of the time which I can hopefully use to train an ML model.

This part has been split into multiple stages.
Each stage adds new strategies and heuristics to test.
Although I'll do lots of testing to improve the strategies and heuristics, the final tests that I'll run at the end of each stage will serve as "official" results for how good each strategy is.
This ensures that I don't have to run the data collection tests that often, since those are computationally expensive.


## Stage 1

The best strategy from this stage succeeded 98.8% of the time.
I ran at least 500 test games for each player using an AWS EC2 Linux c6g.large instance.
See this [CSV file](/results-stage1.csv) for all the results.

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
At later stages of the game, this backfires because there are some situations where losing is possible no matter what moves are chosen, causing the player to simply panic and move left.
That might explain why the `corner-mnmx` strategy is actually [less successful](/results/stage2/corner-mnmx.csv) at `d=4` than `d=3`.

I had an overflow bug in my implementation during the original test run that I didn't discover until most of the data had been collected.
This was fixed by rolling back the project locally to the code that was used for the data collection (since I'd made a lot of improvements since then) and re-running the tests with the fixed code.


### Expectimax Optimization
This strategy is used by the best two traditional solvers I found ([here](https://github.com/nneonneo/2048-ai) and [here](https://github.com/MaartenBaert/2048-ai-emscripten)), which completely blow all the other traditional solvers out of the water.

Expectimax is similar to minimax, but instead of assuming that the computer will place at the worst possible position, the expected value based on the probabilities of the computer's choice is maximized.
All the most successful AIs I found seem to use expectimax.
This stage's implementation of expectimax also doesn't implement any pruning or caching.


## Stage 3
I'd originally decided to drop the less successful strategies from Stage 3 onwards, but I realized that those can be useful to ensure the game implementation still works the same.
It's also interested to see how well each strategy works, even if it's not the most optimal.

I essentially overhauled the entire project structure for this stage.
I also changed the VM I used for testing from an AWS EC2 Amazon Linux c6g.large to an Ubuntu c6g.xlarge, so everything is running faster.

I had to restart the testing program twice after discovering bugs in it.
For Stage 4, I plan to add unit tests and a procedure to run preliminary tests before running the full amount of games.

### Evaluation Caching
The expectimax strategy uses [Google SparseHash](https://github.com/sparsehash/sparsehash) to cache evaluations of board states.
To keep the memory usage from spiking to several gigabytes (the highest I got during testing was 8GB), old evaluations are put in a queue and deleted after a few moves.

### Alpha-Beta Pruning
The minimax strategy now implements [fail-soft alpha-beta pruning](https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning).
No caching is used, since caching would require storing the alpha and beta values of an evaluation along with the evaluation itself.
It's possible that this would still be faster, but for now I haven't tried it.

### Heuristics
I added two heuristics designed to make the AI build tiles against the wall.
This is supposed to replicate the human strategy (which I think is called Snake Chain Formation?) of keeping an "immovable subset" of large tiles against a wall.

### Transition from functional to object-oriented
Previously, each strategy was in its own namespace.
I had decided to avoid classes in the hope that it would be simpler and slightly faster.
Since each `player` function didn't rely on any external state, this was fine.

But once the evaluation cache was added, a global data structure needed to be attached to a specific player.
This would also avoid a single cache from being used by multiple separate games during testing, which happens in parallel.
So the game implementation and each AI implementation was converted to be object-oriented.

While making these changes, I discovered that previous results where testing was done in parallel may have overreported the runtime.
The game implementation initially used a single RNG instance across all threads, which slowed down the players while they waited for the RNG instance to become available.
This was fixed when the game implementation became object-oriented and each instance of `GameSimulator` got its own RNG instance.

### Depth Picker
Both the minimax and expectimax strategies take a long time to evaluate boards with lots of empty tiles, but those positions are usually not very important.
I added a depth picker, which decreases the depth to search for boards with few tiles and increases the depth on boards that are nearly full or have a high number of distinct tiles.

### Optimizations
I added a simple [benchmark](/benchmark.cpp) program to provide some direction in speeding up the game implementation.
I also added `-funroll-loops` to the compilation flags, which sped things up.
Link-time optimization (`-flto`) and CPU-specific tuning (`mcpu=native`) flags were added as well, but the speedup was not particularly large.
Lookup tables for the game implementation are now calculated at compile time.

### Website
While waiting for the testing of Stage 2 to finish, I put together a [website demo](https://qpwoeirut.github.io/2048/).
Besides being a good way to show my work, it also provided a good visual tool for testing new strategies and heuristics.

### Other
* All testing is now run in parallel, and the number of threads used has been lowered significantly.
* [tester.cpp](/tester.cpp) now records the sum of times that each thread is alive, in order to approximate the amount of time that would have been spent if testing had been single-threaded.
* Several other statistics are recorded as well: total score, median score, total # of moves, and median # of moves.
* All the files that don't have an entrypoint have been converted into header files with include guards.
* The [results](/results) directory now stores all previous testing results.


## Other Ideas
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

| Type   | Link                                                                                            | Description                                                  | 2048                | 4096                | 8192  | 16384 | 32768 | 65536 |
|--------|-------------------------------------------------------------------------------------------------|--------------------------------------------------------------|---------------------|---------------------|-------|-------|-------|-------|
| Non-ML | [macroxue's solver](https://github.com/macroxue/2048-ai)                                        | expectimax with a large lookup table for difficult positions | -                   | -                   | 99.7% | 98.7% | 80.5% | 3.5%  |
| ML     | [moporgic's solver](https://github.com/moporgic/TDL2048)                                        | machine learning methods, including TD(0), TD(λ), n-step TD  | -                   | -                   | 99.8% | 98.8% | 72%   | 0.02% |
| ML     | [aszczepanski's solver](https://github.com/aszczepanski/2048)                                   | expectimax with n-tuple network trained with TD(λ) ML        | -                   | -                   | 98%   | 97%   | 69%   | -     |
| Non-ML | [nneonneo's solver](https://github.com/nneonneo/2048-ai)                                        | expectimax with several heuristics, weights tuned by CMA-ES  | 100%                | 100%                | 100%  | 94%   | 36%   | -     |
| Non-ML | [MaartenBaert's solver](https://github.com/MaartenBaert/2048-ai-emscripten)                     | expectimax                                                   | -                   | -                   | -     | -     | -     | -     |
| ML     | [tnmichael309's solver](https://github.com/tnmichael309/2048AI)                                 | expectimax with n-tuple network trained with TD(λ) ML        | 100%                | 99.8%               | 99.5% | 93.6% | 33.5% | -     |
| Non-ML | [VictorGavrish's solver](https://github.com/VictorGavrish/ai2048)                               | expectimax with hand-tuned heuristics                        | 100%                | 100%                | 100%  | 93%   | 31%   | -     |
| Non-ML | [ziap's solver](https://github.com/ziap/2048-wasm)                                              | expectimax with hand-tuned heuristics                        | 100%                | 100%                | 98%   | 85%   | 12%   | -     |
| Non-ML | [kcwu's solver](https://github.com/kcwu/2048-c)                                                 | expectimax with helper minimax search to avoid dying         | 100%                | 100%                | 96%   | 67%   | 2%    | -     |
| Non-ML | [ronzil's solver](https://github.com/ronzil/2048-ai-cpp)                                        | Monte Carlo tree search                                      | 100%                | 70%                 | 1%    | -     | -     | -     |
| Non-ML | [ovolve's solver](https://github.com/ovolve/2048-AI)                                            | minimax with iterative deepening and alpha-beta pruning      | "about 90%"         | -                   | -     | -     | -     | -     |
| Non-ML | [acrantel's solver](https://github.com/acrantel/2048)                                           | expectimax                                                   | "approximately 80%" | "approximately 25%" | -     | -     | -     | -     |
| Non-ML | [datumbox's solver](https://github.com/datumbox/Game-2048-AI-Solver)                            | minimax                                                      | "about 70-80%"      | -                   | -     | -     | -     | -     |
| Non-ML | [vpn1997's solver](https://github.com/vpn1997/2048-Ai)                                          | expectimax                                                   | 60%                 | -                   | -     | -     | -     | -     |
| Non-ML | [xtrp's solver](https://github.com/xtrp/jupiter)                                                | Monte Carlo tree search                                      | "~60%"              | -                   | -     | -     | -     | -     |
| ML     | [navjindervirdee's solver](https://github.com/navjindervirdee/2048-deep-reinforcement-learning) | deep reinforcement learning                                  | 10%                 | 0.05%               | -     | -     | -     | -     |
| ML     | [mmcenta's solver](https://github.com/mmcenta/left-shift)                                       | Deep Q-Learning                                              | 3.8%                | -                   | -     | -     | -     | -     |
