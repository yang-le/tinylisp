#define _CRT_SECURE_NO_WARNINGS
#include "primitive.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char buf[63];
static char see = ' ';

void look()
{
	int c = getchar();
	see = c;
	if (c == EOF)
		exit(0);
}
int seeing(char c) { return c == ' ' ? see > 0 && see <= c : see == c; }
char get()
{
	char c = see;
	look();
	return c;
}

char scan()
{
	int i = 0;
	while (seeing(' '))
		look();
	if (seeing('(') || seeing(')') || seeing('\''))
		buf[i++] = get();
	else
		do
			buf[i++] = get();
		while (i < sizeof(buf) - 1 && !seeing('(') && !seeing(')') && !seeing(' '));
	buf[i] = 0;
	return *buf;
}

box parse();
box read()
{
	scan();
	return parse();
}

box list()
{
	box x;
	if (scan() == ')')
		return NIL;
	if (!strcmp(buf, "."))
	{
		x = read();
		scan();
		return x;
	}
	x = parse();
	return cons(x, list());
}

box quote()
{
	static int64_t quote_ = (int64_t)"quote";
	return cons(tbox(quote_, TSYM), cons(read(), NIL));
}

box atomic()
{
	double d;
	int i;
	int n;

	if (sscanf(buf, "%d%n", &i, &n) > 0 && !buf[n])
		return tbox(i, TNUM);

	if (sscanf(buf, "%lg%n", &d, &n) > 0 && !buf[n])
		return *(box*)(&d);

	int64_t s = (int64_t)strdup(buf);
	return tbox(s, TSYM);
}

box parse()
{
	return *buf == '('	  ? list()
		   : *buf == '\'' ? quote()
						  : atomic();
}

void print(box);
void printlist(box t)
{
	for (putchar('(');; putchar(' '))
	{
		print(car(t));
		t = cdr(t);
		if (t.type == TNIL)
			break;
		if (t.type != TCONS)
		{
			printf(" . ");
			print(t);
			break;
		}
	}
	putchar(')');
}

void print(box x)
{
	if (x.type == TNIL)
		printf("#f");
	else if (x.type == TSYM)
		printf("%s", get_str(x));
	else if (x.type == TPRIM)
		printf("<%d>", get_int(x));
	else if ((x.type == TCONS) || (x.type == TCLOS) || (x.type == TMACR))
		printlist(x);
	else if (x.type == TNUM)
		printf("%d", get_int(x));
	else
		printf("%.10lg", get_double(x));
}

int main()
{
	init_primitive();
	while (1)
	{
		printf("\n> ");
		print(eval(read(), env));
	}
	return 0;
}
