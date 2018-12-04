/*
 * main.c
 *
 *  Created on: Nov 23, 2018
 *      Author: yoram
 *
 *      Sudoku solver.
 *      inout file format:
 *      . . 4 9 3 . . . .
 *		1 5 . . . . . 8 6
 *		. . . . . 1 . 2 9
 *      4 6 . . . 5 . 1 .
 *      . . . 7 . . 9 4 3
 *      . 9 2 4 1 . . . .
 *      . . . 8 . 4 1 . 7
 *      5 . 1 . 2 . . . .
*       6 . . . . 3 5 . .
*
*       usage: sudokur -f inFile
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "bitsop.h"
#include "matrix.h"

int main (int argc, char **argv)
{
	char  opt, *fname;
	FILE *fp = NULL;

	// get commad line input
	while ((opt = getopt(argc, argv, "f:l")) > 0)
		switch (opt)
		{
		case 'f':
			fname = optarg;
			fp = fopen(fname, "r");
			break;
		case 'l':
			loging = 1;
			break;

		default:
			printf("wrong input\n");
			return -1;
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

	if (!solveBoard(pBlk, 0, 0, 0))
		printf("failed to find solution\n");

	printStat(pBlk);
	free(pBlk);



	if (fp != stdin)
		fclose(fp);

	return 0;
}
