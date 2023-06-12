#pragma once

#include <stdint.h>

typedef struct link
{
	struct link *prev;
	struct link *next;
} link;

typedef struct
{
	int64_t ptr : 48;
	uint64_t type : 16;
/* we have 3 bits for types, that is the 48 - 50 bit */
#define TSYM 0x7FF8
#define TPRIM 0x7FF9
#define TCONS 0x7FFA
#define TCLOS 0x7FFB
#define TMACR 0x7FFC
#define TNUM 0x7FFD
} box;

// typedef struct {
//	link l;
//	box c;
// } atom;
//
// typedef struct {
//	link l;
//	box car;
//	box cdr;
// } list;

#define tbox(a, t) (box){(a), (t)}
#define get_int(a) ((int32_t)a.ptr)
#define get_double(a) (*(double *)&a)
#define get_str(a) ((char *)(intptr_t)a.ptr)
#define car(a) ((a.type == TCONS || a.type == TCLOS || a.type == TMACR) ? (*(box *)(intptr_t)a.ptr) : ERR)
#define cdr(a) ((a.type == TCONS || a.type == TCLOS || a.type == TMACR) ? (*((box *)(intptr_t)a.ptr + 1)) : ERR)
#define nil(a) equ(a, NIL)

int equ(box a, box b);
box cons(box a, box b);
box pair(box v, box x, box e);
box closure(box v, box x, box e);
box macro(box v, box x);
