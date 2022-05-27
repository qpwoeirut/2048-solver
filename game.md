# 2048 game implementation

## Structure
In order to make multithreading easier, `GameSimulator` is its own class.
This allows each instance to have its own RNG, which avoids the issue of multiple threads trying to access the same generator.
A lot of the precomputation (such as row shift lookup tables) is done at compile time.

## Game Logic
The board is represented as a 64-bit unsigned integer.
Since the 4x4 board has 16 tiles, that leaves 4 bits, or 16 values, for each tile.
The highest possible theoretical tile is 2<sup>17</sup>, or 131072.
But this tile can only be created if a 4 spawns in a very specific board configuration, so it can be ignored for our purposes.

The second-highest tile does present a bit of an issue though.
2<sup>16</sup> would be the 17th possible value, since 2<sup>0</sup> will represent the empty tile, but there's only space for 16.
The (rather inelegant) solution here is to simply ignore that one too.
If we ever get to that situation, then this project is already much more successful than I'd expect.

Moves will be calculated one row at a time.
If the move is left or right, a direct lookup table of each row (which has a size of 2<sup>16</sup>) will suffice.
If the move is up or down, the board will be transposed, the lookup table will be used, and then the board will be transposed again.

The transposition code is taken from [nneonneo's project](https://github.com/nneonneo/2048-ai/blob/master/2048.cpp#L38-L48) since I do not want to figure it out myself.
