# Model File Format

The saved model is stored in a text file of lines.
Internally, only a large array of floats needs to be saved.
The naive approach of storing every float leads to very large files.
Since most values in the array are zero, only the values that are nonzero are saved.

There are two types of lines in the file format.
The first specifies which index of the array is represented by that line, in the form `idx=val`.
The second holds only a value.
Its index is determined by the index of the line before it.
The file must start with the first line type.

