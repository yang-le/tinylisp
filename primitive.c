#include "primitive.h"
#include <string.h>
#include <stdlib.h>

box NIL;
box ERR;
box env;

static box T;

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
	return (car(t).type != TCONS && car(t).type != TCLOS) ? T : NIL;
}

box f_eq(box t, box e)
{
	t = evlis(t, e);
	return equ(car(t), car(cdr(t))) ? T : NIL;
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
	while (t.type != TNIL && eval(car(car(t)), e).type == TNIL)
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
	PRIM(env),
	PRIM(macro)
#undef PRIM
};

void init_primitive()
{
	static int64_t t = (int64_t)"#t";
	static int64_t nil = (int64_t)"";
	static int64_t err = (int64_t)"#err";
	T = tbox(t, TSYM);
	NIL = tbox(nil, TNIL);
	ERR = tbox(err, TSYM);

	env = pair(T, T, NIL);
	for (unsigned i = 0; i < sizeof(prims) / sizeof(prims[0]); ++i) {
		int64_t name = (int64_t)prims[i].name;
		env = pair(tbox(name, TSYM), tbox(i, TPRIM), env);
	}
}

box bind(box v, box t, box e)
{
	return v.type == TNIL ? e :
		v.type == TCONS ? bind(cdr(v), cdr(t), pair(car(v), car(t), e)) : pair(v, t, e);
}

box reduce(box f, box t, box e)
{
	return eval(cdr(car(f)), bind(car(car(f)), evlis(t, e), cdr(f).type == TNIL ? env : cdr(f)));
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

box eval(box x, box e)
{
	return x.type == TSYM ? assoc(x, e) :
		x.type == TCONS ? apply(eval(car(x), e), cdr(x), e) : x;
}

// void print(box x);
// box eval(box x, box e)
// {
// 	box y = step(x, e);
// 	print(x); printf(" => "); print(y); putchar('\n');
// 	return y;
// }