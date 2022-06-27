# 2048 Solver

## Demo
### Website
To watch the AI yourself, go to this [website](https://qpwoeirut.github.io/2048/) or check out this [repository](https://github.com/qpwoeirut/2048).

### Selenium Remote Controls
You can also run this with Selenium, although that's not recommended.
The instructions for that are [here](/demo/selenium/README.md).


## Summary
This project is a set of AI strategies that play the popular [2048 tile-merging game](https://github.com/gabrielecirulli/2048).
See the [roadmap](/roadmap.md) for details on the project.

Although most of the ideas I'll use are taken directly from others' projects, I want to try writing most of the source code myself.
There are some implementation ideas that I'm taking as well (such as the bitwise board transposition [code](https://github.com/nneonneo/2048-ai/blob/master/2048.cpp#L38-L48)), but most of the code should be my own.


## Structure
The code for simulating a game is in the `GameSimulator` class under [game.hpp](/game.hpp).
[util.hpp](/util.hpp) stores helpful utilities for the heuristic and player functions.

Each strategy is in the [strategy](/strategies) directory.
All strategies implement a function which provides a move when given a board. 
Some strategies have parameters and heuristic functions or secondary strategies that are passed in.
All heuristics are in [heuristics.hpp](/heuristics.hpp).

[tester.cpp](/tester.cpp) simulates games for each solver and write the results into the [results](/results) directory as a CSV file.
Giving each solver its own file means that I don't have to rerun every solver simulation if I only need to test one solver.
These CSV files are then combined by [collate.py](/results/collate.py).
In the future I might write a program to plot/visualize the data, especially for comparing different parameters on certain solvers.
For now, the best visual I have is a [Google Sheet](https://docs.google.com/spreadsheets/d/1wgqyGy-4k6yAPCt85HvR2o7Bwf5hlHvNM6eBblyt5Kg/edit) with conditional formatting.


## Testing
Each test runs at least 500 games in order to try and minimize random variance.
In general, the number of games run is increased until it takes a least a half minute to complete.
As a result, some of the faster solvers (such as the random strategy) run hundreds of thousands of games.

All game tests are run in parallel using C++'s `std::async`.

For Stages 1 and 2, games were run on an AWS EC2 Amazon Linux c6g.large instance.
From Stage 3 onwards, games were run on an AWS EC2 Ubuntu c6g.xlarge instance.


## Results
The best strategy right now is the [expectimax strategy](/strategies/ExpectimaxDepthStrategy.hpp) with the [corner building heuristic](/heuristics.hpp).
On the most recent set of tests, it reaches 4096 99.4% of the time, gets the 8192 tile with a 91.8% success rate, and reaches 16384 in 34.6% of its games.

The [results file](/results-stage3.csv) has the latest statistics for all tested strategies.


## Caveats
The [Google sparsehash](https://github.com/sparsehash/sparsehash) repository needs to be installed.
