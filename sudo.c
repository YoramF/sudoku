/*
 * sodu.c
 *
 *  Created on: Nov 23, 2018
 *      Author: yoram
 *
 *      Sudoku solver.
 *      inout file format:
 *      . . 4 9 3 . . . .
 *      1 5 . . . . . 8 6
 *      . . . . . 1 . 2 9
 *      4 6 . . . 5 . 1 .
 *      . . . 7 . . 9 4 3
 *      . 9 2 4 1 . . . .
 *      . . . 8 . 4 1 . 7
 *      5 . 1 . 2 . . . .
*       6 . . . . 3 5 . .
*
*       usage: sudokur -f inFile -l[1/2]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define isBitSet(b, c)		((1<<(b) & (c)))
#define setBit(b, c)		((1<<(b)) | (c))
#define clearBit(b, c)		((~(1<<(b))) & (c))

typedef struct _add
{
	char raw;
	char col;
} addR;

typedef struct _brd
{
	char                board [9][9];   // the sudoku board
	char                score [9][9];	// score board
	unsigned short int  bitMask [9][9];	// bit mask for representing possible values
	char                cells;
} brdR;

addR            blks [3][3];
brdR            *pBlk;
long long int   iter = 0, stack = 0, deepest = 0; // for statistic printouts
int	            loging = 0;


void printBoard (char b[9][9])
{
	int i,j;

	for (i = 0; i < 9; i++)
	{
		for (j = 0; j < 9; j++)
			printf("%d ", b[i][j]);

		printf("\n");
	}
}

void printStat(brdR *b, int r, int c, int v)
{
	switch (loging)
	{
	case 2:
		printBoard(b->board);
	case 1:
		printf("Iterations: %I64d, (stk:%I64d), (recd:%I64d), (r:%d), (c:%d), (v:%d) Non empty cells: %d\n", iter, stack, deepest, r, c, v, b->cells);
	default:
		break;
	}
}

// return number of set bits in word
char countSetBits (unsigned short int num)
{
	char count = 0;

	while (num)
	{
		if (num&1)
			count++;
		num >>= 1;
	}
	return count;
}


// Once we set a cell with nre value, all cells on same raw, col must be updated
// so that this value is not a valid value for them
void updateLines (brdR *b, const int r, const int c, const char v)
{
	int i;

	for (i = 0; i < 9; i++)
	{
		b->bitMask[r][i] = clearBit(v, b->bitMask[r][i]);
		b->bitMask[i][c] = clearBit(v, b->bitMask[i][c]);

		if (b->score[r][i] > 0)
			b->score[r][i] = countSetBits(b->bitMask[r][i]);
		if (b->score[i][c] > 0)
			b->score[i][c] = countSetBits(b->bitMask[i][c]);
	}
}

// Once we set a cell with nre value, all cells on same block must be updated
// so that this value is not a valid value for them
void updateBlock (brdR *b, const int r, const int c, const char v)
{
	int i1, i2, j1, j2;
	addR vertex = blks[r/3][c/3];

	i2 = vertex.raw + 3;
	j2 = vertex.col + 3;

	for (i1 = vertex.raw; i1 < i2; i1++)
		for (j1 = vertex.col; j1 < j2; j1++)
			if (b->score[i1][j1] > 0)
			{
				b->bitMask[i1][j1] = clearBit(v, b->bitMask[i1][j1]);
				b->score[i1][j1] = countSetBits(b->bitMask[i1][j1]);
			}
}

// get next available value from given bitmask
char getVal(unsigned short int bits)
{
	char i;

	for (i = 0; i < 10; i++)
		if (isBitSet(i, bits))
			break;

	return (i < 10? i: 0);
}


//set new value in a given cell
int setNewValue (brdR *b, int r, int c, char v)
{
	iter++;
	// check that selected location is free, otherwise return error
	if ((b->board[r][c] == 0) && (isBitSet(v, b->bitMask[r][c])))
	{
		b->board[r][c] = v;
		b->score[r][c] = -1;

		updateLines(b, r, c, v);
		updateBlock(b, r, c, v);

		b->cells++;

		if (loging)
			printStat(b, r, c, v);

		return 1;
	}
	else
		return 0;
}

// find the first non set cell location
// maxOpt indicates max value options allowed for the selected cell
int findFirstNonSet (brdR *b, int *r, int *c, int maxOpt)
{
	int i, j, score;

	// if board is complete no next cell for update
	if (b->cells == 81)
		return 0;

	for (i = 0; i < 9; i++)
		for (j = 0; j < 9; j++)
		{
			score = b->score[i][j];
			if(score > -1)
			{
				if ((score > 0) && (score <= maxOpt))
				{
					*r = i;
					*c = j;
					return 1;
				}
				else if (score == 0)
					// dead-end
					return 0;
			}
		}

	return 0;
}


int initB (FILE *fp)
{
	int i, j, k;
	char input [81];
	char c;
	addR vert;

	if ((pBlk = calloc(1, sizeof(brdR))) == NULL)
			return 0;

	for (i=0; i<3; i++)
		for(j=0; j<3; j++)
		{
			vert.raw = i*3;
			vert.col = j*3;
			blks[i][j] = vert;
		}

	for (i=0; i < 9; i++)
		for (j = 0; j < 9; j++)
		{
			pBlk->bitMask[i][j] = 0x3fe;
			pBlk->score[i][j] = 9;
		}

	// fill board with initial values
	// read these values from stdin
	// no imput validation !!!
	for (i = 0; i < 9; i++)
	{
		fgets(input, sizeof(input), fp);
		for (j = 0, k = 0; j < 9; j++)
		{
			c = input[k++] - '0';
			c = ((c >= 1) && (c <= 9)? c: 0);

			if (c > 0)
				if(!setNewValue(pBlk, i, j, c))
				{
					printf("Invalid board, can't add value: %d in (r:%d) (c:%d)\n", c, i+1, j+1);
					return 0;
				}

			k++;
		}
	}

	return 1;
}


// this is the recursive function that is used to solve the board
// the function will update the given board only if it managed to sove it.
// otherwise it is using a local copy to try and fing solution using recursion calls
int solveBoard (brdR *board, const int raw, const int col, const char value)
{
	int r , c;
	brdR *b;
	int changed;
	char v;
	unsigned short bitmask;

	stack++;
	deepest = (deepest > stack? deepest: stack);

	// create local copy of board and sent it to solvBoard()
	if ((b = malloc(sizeof(brdR))) == NULL)
		return 0;
	memcpy(b, board, sizeof(brdR));

	// if we call solveBoard with Value > 0 we need first to change the input board to have this value
	// as final value for this recursion
	if (value > 0)
		if (!setNewValue(b, raw, col, value))
		{
			printf("fatal Error!\n");
			return 0;
		}


	// try to solve board assuming we now might have new cells with single possible value option
	changed = 1;
	while (changed && (b->cells < 81))
	{
		changed = findFirstNonSet(b, &r, &c, 1);
		if	(changed)
			changed = setNewValue(b, r, c, getVal(b->bitMask[r][c]));
	}

	if (b->cells == 81)
	{
		// found solution - exit recursion
		memcpy(board, b, sizeof(brdR));
		free(b);
		stack--;
		return 1;
	}


	// if we get here, it means we could not find any other cells with single value option.
	// now scan the board for all cells with more value options
	if (findFirstNonSet(b, &r, &c, 9))
	{
		bitmask = b->bitMask[r][c];
		while ((v = getVal(bitmask)))
		{
			// recursive call !!
			if (!solveBoard(b, r, c, v))
				bitmask = clearBit(v, bitmask);
			else
			{
				// recursive call solved the board. we need to updated the input board and return
				memcpy(board, b, sizeof(brdR));
				free(b);
				stack--;
				return 1;
			}
		}
	}

	// if we get here it means we did not find solution for this recursive branch
	free(b);
	stack--;

	return 0;
}



int main (int argc, char **argv)
{
	char  opt, *fname = NULL;
	FILE *fp = NULL;

	// get commad line input
	if (argc > 1)
	{
		while ((opt = getopt(argc, argv, "f:l:")) > 0)
			switch (opt)
			{
			case 'f':
				fname = optarg;
				fp = fopen(fname, "r");
				break;
			case 'l':
				loging = atoi(optarg);
				break;

			default:
				printf("wrong input\n");
				return -1;
			}
		}

	if (!fp)
	{
		printf("error opening file %s, error: %s\n Change to stdin\n", fname, strerror(errno));
		fp = stdin;
	}


	if (!initB(fp))
	{
		printf("failed to initialize board\n");
		return -1;
	}

	printf("Input board:\n");
	printBoard(pBlk->board);
	printf("\n");

	iter=0;
	if (!solveBoard(pBlk, 0, 0, 0))
		printf("failed to find solution\n");

	printBoard(pBlk->board);
	printf("Iterations: %I64d, (recd:%I64d)\n", iter, deepest);
	free(pBlk);



	if (fp != stdin)
		fclose(fp);

	return 0;
}
