# 2048 Solver

## Quick Start Instructions
To set up the demo, run `pip install .` at the top level of this repository.
By default, the demo runs in the Firefox browser.
To run in Firefox, you may need to install the `geckodriver` executable.

The code in [main.py](/demo/selenium/main.py) should be editable so that it works for any webdriver-supported browser.
It's worth noting that it takes a while for the page to load up in Chrome, which is why Firefox is the default.

## Summary
This project is intended to "solve" the popular [2048 tile-merging game](https://github.com/gabrielecirulli/2048).
See the [roadmap](/roadmap.md) for details on the project.

Although most of the ideas I'll use are taken directly from others' projects, I want to try writing most of the source code myself.
There are some implementation ideas that I'm taking as well (such as the bitwise board transposition [code](https://github.com/nneonneo/2048-ai/blob/master/2048.cpp#L38-L48)), but most of the code should be my own.


## Structure
The code for simulating a game is in the [game](/game) directory under the `game` namespace.
It has a [game.cpp](/game/game.cpp) and a [util.cpp](/game/util.cpp).

Each strategy is in the [strategy](/strategies) directory.
All strategies implement a function which provides a move when given a board. 
Some strategies have parameters and heuristic functions or secondary strategies that are passed in.
All heuristics are in [heuristics.cpp](/heuristics.cpp).

I'm not using classes because I couldn't figure out an easy way to pass around class member functions and I figured that there would probably be some avoidable overhead from instantiating classes.
So instead each strategy just gets its own namespace.

[tester.cpp](/tester.cpp) simulates games for each solver and write the results into the [results](/results) directory as a CSV file.
Giving each solver its own file means that I don't have to rerun every solver simulation if I only need to test one solver.
These CSV files are then combined by [collate.py](/results/collate.py).
In the future I might write a program to plot/visualize the data, especially for comparing different parameters on certain solvers.


## Testing
Each test runs at least 500 games in order to try and minimize random variance.
In general, the number of games run is increased until it takes a least a half minute to complete.
As a result, some of the faster solvers (such as the random strategy) run hundreds of thousands of games.

The slower solvers run games in parallel using C++'s `std::async`.

Games are run on an AWS EC2 Linux c6g.large instance.

## Results
The best strategy right now is the [Monte Carlo strategy](/solvers/monte_carlo.cpp). When the `trials` parameter is set to 2000, it reaches the 2048 tile 98.8% of the time and the 4096 tile 56.6% of the time.

The [results file](/results.csv) has the statistics for all tested strategies.
