/*
 * matrix.h
 *
 *  Created on: Nov 23, 2018
 *      Author: yoram
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include <stdio.h>

typedef struct _add
{
	char raw;
	char col;
} addR;

typedef struct _brd
{
	char				board [9][9];			// the sudoku board
	char 				score [9][9];		    // score board
	unsigned short int 	bitMask [9][9];			// bit mask for representing possible values
	char				cells;
} brdR;

void printBoard (char b[9][9]);
int initB (FILE *fp);
int solveBoard (brdR *board, const int raw, const int col, const char value);
void printStat(brdR *board);

extern long long int	iter;
extern brdR 			*pBlk;
extern int				loging;

#endif /* MATRIX_H_ */
