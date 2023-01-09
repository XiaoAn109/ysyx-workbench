#ifndef __BASIC_FUNC_H__
#define __BASIC_FUNC_H__

#include "common.h"
#include "memory.h"
#include "expr.h"
#include "watchpoint.h"

#include <getopt.h>
#include <time.h>
#include <dlfcn.h>

//verilator basic functions
void step_and_dump_wave();
void single_cycle();
void resetn(int n);
int sim_exit();
void sim_init(int argc, char *argv[]);
void sim_exec(uint64_t n);


#endif
