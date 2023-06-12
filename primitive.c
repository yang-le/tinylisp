#include "primitive.h"
#include <string.h>
#include <stdlib.h>

box NIL;
box ERR;
box env;

static box T;
static box F;

box assoc(box v, box e)
{
	while (e.type == TCONS && !equ(v, car(car(e))))
		e = cdr(e);
	return e.type == TCONS ? cdr(car(e)) : ERR;
}

box evlis(box t, box e)
{
	return t.type == TCONS ? cons(eval(car(t), e), evlis(cdr(t), e)) :
		t.type == TSYM ? assoc(t, e) : NIL;
}

box f_atom(box t, box e)
{
	t = evlis(t, e);
	return (car(t).type != TCONS && car(t).type != TCLOS && car(t).type != TPRIM && car(t).type != TMACR) ? T : F;
}

box f_eq(box t, box e)
{
	t = evlis(t, e);
	return equ(car(t), car(cdr(t))) ? T : F;
}

box f_car(box t, box e)
{
	t = evlis(t, e);
	return car(car(t));
}

box f_cdr(box t, box e)
{
	t = evlis(t, e);
	return cdr(car(t));
}

box f_cons(box t, box e)
{
	t = evlis(t, e);
	return cons(car(t), car(cdr(t)));
}

box f_quote(box t, box _)
{
	return car(t);
}

box f_cond(box t, box e)
{
	while (!nil(t) && equ(eval(car(car(t)), e), F))
		t = cdr(t);
	return eval(car(cdr(car(t))), e);
}

box f_lambda(box t, box e)
{
	return closure(car(t), car(cdr(t)), e);
}

box f_label(box t, box e)
{
	env = pair(car(t), eval(car(cdr(t)), e), env);
	return car(t);
}

box f_env(box _, box e)
{
	return e;
}

box f_macro(box t, box _)
{
	return macro(car(t), car(cdr(t)));
}

struct {
	char* name;
	box(*fn)(box, box);
} prims[] = {
#define PRIM(x) { #x, f_##x }
	PRIM(atom),
	PRIM(eq),
	PRIM(car),
	PRIM(cdr),
	PRIM(cons),
	PRIM(quote),
	PRIM(cond),
	PRIM(lambda),
	PRIM(label),
	PRIM(macro),
#ifdef EXTEND
	PRIM(env),
#endif
#undef PRIM
};

void init_primitive()
{
	static int64_t t = (int64_t)"#t";
	static int64_t f = (int64_t)"#f";
	static int64_t nil = (int64_t)"#nil";
	static int64_t err = (int64_t)"#err";
	T = tbox(t, TSYM);
	F = tbox(f, TSYM);
	NIL = tbox(nil, TSYM);
	ERR = tbox(err, TSYM);

	env = pair(T, T, NIL);
	env = pair(F, F, env);
	env = pair(NIL, NIL, env);
	env = pair(ERR, ERR, env);

	for (unsigned i = 0; i < sizeof(prims) / sizeof(prims[0]); ++i) {
		int64_t name = (int64_t)prims[i].name;
		env = pair(tbox(name, TSYM), tbox(i, TPRIM), env);
	}
}

box bind(box v, box t, box e)
{
	return nil(v) ? e :
		v.type == TCONS ? bind(cdr(v), cdr(t), pair(car(v), car(t), e)) : pair(v, t, e);
}

box reduce(box f, box t, box e)
{
	return eval(cdr(car(f)), bind(car(car(f)), evlis(t, e), nil(cdr(f)) ? env : cdr(f)));
}

box expand(box f, box t, box e)
{
	return eval(eval(cdr(f), bind(car(f), t, env)), e);
}

box apply(box f, box t, box e)
{
	return f.type == TPRIM ? prims[get_int(f)].fn(t, e) :
		f.type == TCLOS ? reduce(f, t, e) :
		f.type == TCONS ? apply(eval(f, e), t, e) :
		f.type == TMACR ? expand(f, t, e) : ERR;
}

#ifdef DEBUG
box step(box x, box e)
#else
box eval(box x, box e)
#endif
{
	return x.type == TSYM ? assoc(x, e) :
		x.type == TCONS ? apply(eval(car(x), e), cdr(x), e) : x;
}

#ifdef DEBUG
void print(box x);
#include <stdio.h>
box eval(box x, box e)
{
	box y = step(x, e);
	print(x); printf(" => "); print(y); putchar('\n');
	return y;
}
#endif
