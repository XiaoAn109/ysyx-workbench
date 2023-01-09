#ifndef __EXPR_H__
#define __EXPR_H__

#include "common.h"
#include <regex.h>

void init_regex();
word_t expr(char *e, bool *success);
#endif
