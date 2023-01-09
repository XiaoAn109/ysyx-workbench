#ifndef __REG_H__
#define __REG_H__

#include "common.h"
#include "verilated_dpi.h"

void dump_gpr();
word_t reg_str2val(const char *s, bool *success);
#endif
