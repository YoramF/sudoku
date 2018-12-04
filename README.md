# sudoku
9x9 sudoku solver in C

A sudoku solver program written in C using bit level and pointer operations
usage: sudokur -f inFile [-l if we want log printouts]

inout file shoudl have the following format
. . 4 9 3 . . . .
1 5 . . . . . 8 6
. . . . . 1 . 2 9
4 6 . . . 5 . 1 .
. . . 7 . . 9 4 3
. 9 2 4 1 . . . .
. . . 8 . 4 1 . 7
5 . 1 . 2 . . . .
6 . . . . 3 5 . .

If theboard is solved the final board will be printed.
In additonal statistics like number of iterations used, recursion level and number of solved cells.
