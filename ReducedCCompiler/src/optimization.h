#pragma once

#define NO_OPTIMIZATION 0

/*
* Enables constants folding.
* Ex:
*       4 + 5 * 3 / 2 - 1
* ----> 4 + 15 / 2 - 1
* ----> 4 + 7 - 1
* ----> 4 + 6
* ----> 10
*/
#define OPTI_CONST_FOLD (1 << 0) 

typedef unsigned char optimization_t;

static inline int is_opti_enabled(optimization_t optimizations, optimization_t opti_code)
{
	return (optimizations & opti_code) != 0;
}
