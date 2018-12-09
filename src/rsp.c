#include "common.h"

#include "reciprocal-table.h"

const uint8_t elparamtab[16][8] = {
	// beg, end, step, (pad)
	{0, 1, 2, 3, 4, 5, 6, 7,},
	{0, 1, 2, 3, 4, 5, 6, 7,}, // TODO: FIND WHAT THIS DOES

	{0, 0, 2, 2, 4, 4, 6, 6,},
	{1, 1, 3, 3, 5, 5, 7, 7,},

	{0, 0, 0, 0, 4, 4, 4, 4,},
	{1, 1, 1, 1, 5, 5, 5, 5,},
	{2, 2, 2, 2, 6, 6, 6, 6,},
	{3, 3, 3, 3, 7, 7, 7, 7,},

	{0, 0, 0, 0, 0, 0, 0, 0,},
	{1, 1, 1, 1, 1, 1, 1, 1,},
	{2, 2, 2, 2, 2, 2, 2, 2,},
	{3, 3, 3, 3, 3, 3, 3, 3,},
	{4, 4, 4, 4, 4, 4, 4, 4,},
	{5, 5, 5, 5, 5, 5, 5, 5,},
	{6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 7, 7, 7, 7, 7,},
};


#define GENERATE_MIPS_RSP
#include "rsp.h"
#undef GENERATE_MIPS_RSP
