/*
 * matrix.c
 *
 *  Created on: Nov 22, 2018
 *      Author: yoram
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bitsop.h"
#include "matrix.h"

addR		 	blks [3][3];
brdR 			*pBlk;
long long int 	iter = 0, stack = 0, deepest = 0; // for statistic printouts
int				loging = 0;

static void printStat(brdR *b, int r, int c, int v)
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


void printBoard (char b[9][9])
{
	int i,j;
	int cc = 0, rc = 0;

	for (i = 0; i < 9; i++)
	{
		for (j = 0; j < 9; j++)
		{
			printf("%d ", b[i][j]);
			if (++cc == 3)
			{
				printf("|");
				cc = 0;
			}
		}
		printf("\n");
		if (++rc == 3)
		{
			printf("---------------------\n");
			rc = 0;
		}
	}
}

// Once we set a cell with nre value, all cells on same raw, col must be updated
// so that this value is not a valid value for them
static void updateLines (brdR *b, const int r, const int c, const char v)
{
	int i;

	for (i = 0; i < 9; i++)
	{
		b->bitMask[r][i] = clearBit(v, b->bitMask[r][i]);
		b->bitMask[i][c] = clearBit(v, b->bitMask[i][c]);

		if (b->score[r][i] > 0)
			b->score[r][i] = 0;
		if (b->score[i][c] > 0)
			b->score[i][c] = 0;
	}
}

// Once we set a cell with nre value, all cells on same block must be updated
// so that this value is not a valid value for them
static void updateBlock (brdR *b, const int r, const int c, const char v)
{
	int i1, i2, j1, j2;
	addR vertex = blks[r/3][c/3];

	i2 = vertex.raw + 3;
	j2 = vertex.col + 3;

	for (i1 = vertex.raw; i1 < i2; i1++)
		for (j1 = vertex.col; j1 < j2; j1++)
			if (b->score[i1][j1] > 0)
			{
				b->score[i1][j1] = 0;
				b->bitMask[i1][j1] = clearBit(v, b->bitMask[i1][j1]);
			}
}

// get next available value from given bitmask
static char getVal(unsigned short int bits)
{
	char i;

	for (i = 0; i < 10; i++)
		if (isBitSet(i, bits))
			break;

	return (i < 10? i: 0);
}

// find the set of possible values in given 3x3 block
static unsigned short int missingNumB (brdR *b, int br, int bc, int dim)
{
	int i1, i2, j1, j2;
	unsigned short int numbers = 0x3FE;
	addR vertex = blks[br][bc];

	i2 = vertex.raw + dim;
	j2 = vertex.col + dim;

	// clear birts for any none empty cell
	for (i1 = vertex.raw; i1 < i2; i1++)
		for (j1 = vertex.col; j1 < j2; j1++)
			numbers = clearBit(b->board[i1][j1], numbers);

	return numbers;
}

// return number of set bits in word
static char countSetBits (unsigned short int num)
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

//set new value in a given cell
static int setNewValue (brdR *b, int r, int c, char v)
{

	iter++;
	// check that selected location is free, otherwise return error
	if (b->board[r][c] == 0)
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
static int findFirstNonSet (brdR *b, int *r, int *c)
{
	int i, j;
	int score;
	unsigned short int bits, bvn;

	// if board is complete no next cell for update
	if (b->cells == 81)
		return 0;

	for (i = 0; i < 9; i++)
		for (j = 0; j < 9; j++)
		{
			if(b->score[i][j] > -1)
			{
				bits = b->bitMask[i][j];
				bvn = missingNumB(b, i/3, j/3, 3);
				bits &= bvn;
				score = countSetBits(bits);
				if (score > 0)
				{
					*r = i;
					*c = j;
					return 1;
				}
				else
					// dead-end
					return 0;
			}
		}

	return 0;
}


// search the input board for the location that points to cell with one possible value
// returns 1 if nextCell was found
// returns 0 if nothing was found.
static int findNextCell (brdR *b, int *r, int *c, char *v)
{
	int i, j;
	int score;
	unsigned short int bits, bvn;

	// if board is complete no next cell for update
	if (b->cells == 81)
		return 0;

	for (i = 0; i < 9; i++)
		for (j = 0; j < 9; j++)
		{
			score = b->score[i][j];
			bits = b->bitMask[i][j];
			if ( score == 0)
			{
				bvn = missingNumB(b, i/3, j/3, 3);
				bits &= bvn;
				score = countSetBits(bits);
			}

			if (score == 1)
			{
				*r = i;
				*c = j;
				*v = getVal(bits);
				return 1;
			}
			else if (score > 1)
				b->score[i][j] = score;					// just update score table
			// reached deadend
			else if (score == 0)
				return 0;
		}
	// if we get here than we did not find any candidate.
	// either because we solved the board or we could not find good candidate
	// If we did not complete the board, we might still find solution using recursive search
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
			pBlk->bitMask[i][j] = 0x3fe;

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
				setNewValue(pBlk, i, j, c);

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
	// as final value for this split location
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
		changed = findNextCell(b, &r, &c, &v);
		if	(changed)
			changed = setNewValue(b, r, c, v);
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
	r = raw;
	c = col;
	if (findFirstNonSet(b, &r, &c))
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

	// if we get here it means we did not find solution
	free(b);
	stack--;

	return 0;
}
