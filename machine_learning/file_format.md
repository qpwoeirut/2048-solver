# Model File Format

The model is stored in a binary file with the following contents.
* the string "qp2048TD0", which acts as a file type identifier.
* three 8-bit integers `m`, `n`, and `c`, representing the number of tuples, the size of each tuple, and tile cap respectively.
* `m * n` 8-bit integers, representing the tuples. Each integer should be a multiple of 4 in the range `[0, 64)`.
* `ceil(m * c^n / 8)` bytes, representing a boolean array of size `m * c^n`. Each entry denotes whether the model's float array is nonzero at that index.
* `k` floats, where `k` is the number of set bits in the above boolean array. These floats are the weights for the model. Each float corresponds to one set bit.

## Old Formats
Old models were stored as a text file representing an entry into a sparse array per line.

