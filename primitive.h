#pragma once

#include "types.h"

extern box NIL;
extern box ERR;
extern box env;

void init_primitive();
box eval(box x, box e);
