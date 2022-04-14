# 2048 Solver

## Quick Start Instructions
To set up the demo, run `pip install .` at the top level of this repository.
By default, the demo runs in the Firefox browser.
To run in Firefox, you may need to install the `geckodriver` executable.

The code in [main.py](/demo/selenium/main.py) should be editable so that it works for any webdriver-supported browser, although I've had some issues when trying to use Chrome.

## Summary

This project is intended to "solve" the popular [2048 tile-merging game](https://github.com/gabrielecirulli/2048).
See the [roadmap](/roadmap.md) for details on the project.

Although most of the ideas I'll use are taken directly from others' projects, I want to try writing most of the source code myself.
There are some implementation ideas that I'm taking as well (such as the bitwise board transposition [code](https://github.com/nneonneo/2048-ai/blob/master/2048.cpp#L38-L48)), but most of the code should be my own.


## Structure
The code for simulating a game is in the [game](/game) directory under the `game` namespace.
It has a [game.cpp](/game/game.cpp) and a [utils.cpp](/game/utils.cpp).

The solvers are in the [solvers](/solvers) directory.
All solvers implement a function which provides a move when given a board.
I'm not using classes because I couldn't figure out an easy way to pass around class member functions and I figured that there would probably be some avoidable overhead from instantiating classes.
So instead each solver just gets its own namespace.

[tester.cpp](/tester.cpp) simulates games for each solver and write the results into the [results](/results) directory as a one-line CSV file.
Giving each solver its own file means that I don't have to rerun every solver simulation if I only need to test one solver.
I plan to write a Python program to collate each resulting CSV file into a centralized file soon.
There are also hopes of writing a program to plot/visualize the data, especially for comparing different parameters on certain solvers.

## Testing
Each test runs at least 100 games in order to try and minimize random variance.
In general, the number of games run is increased until it takes a least a few seconds to complete.
As a result, some of the faster solvers (such as the random strategy) run hundreds of thousands of games.

The slow solvers run games in parallel using C++'s `std::async`.

All games are run locally on my computer.
The recorded time taken in the [results](/results) directory will vary, since my computer's environment also varies.
In particular, some runs took place while other things were also happening on my computer, and some games were also run while I was cooling my computer with ice packs.

