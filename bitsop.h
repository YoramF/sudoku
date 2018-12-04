/*
 * bitsop.h
 *
 *  Created on: Nov 22, 2018
 *      Author: yoram
 *
 *      bits manipulations
 */

#define isBitSet(b, c)		((1<<(b) & (c)))
#define setBit(b, c)		((1<<(b)) | (c))
#define clearBit(b, c)		((~(1<<(b))) & (c))





