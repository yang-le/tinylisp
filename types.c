#include "types.h"
#include "primitive.h"
#include <stdlib.h>
#include <string.h>

box cons(box x, box y)
{
	box *b = (box *)malloc(sizeof(box) * 2);
	b[0] = x;
	b[1] = y;
	int64_t c = (int64_t)b;
	return tbox(c, TCONS);
}

box pair(box v, box x, box e)
{
	return cons(cons(v, x), e);
}

box closure(box v, box x, box e)
{
	box c = pair(v, x, equ(e, env) ? NIL : e);
	c.type = TCLOS;
	return c;
}

box macro(box v, box x)
{
	box m = cons(v, x);
	m.type = TMACR;
	return m;
}

int equ(box a, box b)
{
	if (a.type == TSYM && b.type == TSYM)
		return !strcmp(get_str(a), get_str(b));

	return *(uint64_t*)&a == *(uint64_t*)&b;
}
