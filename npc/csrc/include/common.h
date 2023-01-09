#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "verilated.h"
#include "verilated_vcd_c.h"
#include "Vtop.h"
#include "Vtop__Dpi.h"

#define BASE_ADDR 0x80000000

typedef uint64_t word_t;
typedef int64_t sword_t;
typedef uint64_t vaddr_t;
typedef uint64_t paddr_t;


// ----------- log -----------

#define ANSI_FG_BLACK   "\33[1;30m"
#define ANSI_FG_RED     "\33[1;31m"
#define ANSI_FG_GREEN   "\33[1;32m"
#define ANSI_FG_YELLOW  "\33[1;33m"
#define ANSI_FG_BLUE    "\33[1;34m"
#define ANSI_FG_MAGENTA "\33[1;35m"
#define ANSI_FG_CYAN    "\33[1;36m"
#define ANSI_FG_WHITE   "\33[1;37m"
#define ANSI_BG_BLACK   "\33[1;40m"
#define ANSI_BG_RED     "\33[1;41m"
#define ANSI_BG_GREEN   "\33[1;42m"
#define ANSI_BG_YELLOW  "\33[1;43m"
#define ANSI_BG_BLUE    "\33[1;44m"
#define ANSI_BG_MAGENTA "\33[1;35m"
#define ANSI_BG_CYAN    "\33[1;46m"
#define ANSI_BG_WHITE   "\33[1;47m"
#define ANSI_NONE       "\33[0m"

#define ANSI_FMT(str, fmt) fmt str ANSI_NONE
#define FMT_WORD "0x%016lx"

#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)

#define log_write(...) \
	do { \
		extern FILE* log_fp; \
		fprintf(log_fp, __VA_ARGS__); \
		fflush(log_fp); \
	} while (0)


#define _Log(...) \
  do { \
    printf(__VA_ARGS__); \
		extern FILE* log_fp; \
		if(log_fp != stdout) { \
			log_write(__VA_ARGS__); \
		} \
  } while (0)

#define Print(fmt, format, ...) \
    _Log(ANSI_FMT(format, fmt) "\n", ## __VA_ARGS__)

#define Log(format, ...) \
    _Log(ANSI_FMT("[%s:%d %s] " format, ANSI_FG_BLUE) "\n", \
        __FILENAME__, __LINE__, __func__, ## __VA_ARGS__)

#define Error(format, ...) \
    _Log(ANSI_FMT("[%s:%d %s] " format, ANSI_FG_RED) "\n", \
        __FILENAME__, __LINE__, __func__, ## __VA_ARGS__)

#define Assert(cond, format, ...) \
	do { \
		if (!(cond)) { \
			_Log(ANSI_FMT(format, ANSI_FG_RED) "\n", ## __VA_ARGS__); \
      assert(cond); \
    } \
  } while (0)

#define panic(format, ...) Assert(0, format, ## __VA_ARGS__)

#define TODO() panic("please implement me")

//npc_state

enum { NPC_RUNNING, NPC_STOP, NPC_END, NPC_ABORT, NPC_QUIT };


typedef struct {
  int state;
  vaddr_t halt_pc;
  uint32_t halt_ret;
} NPC_state;

typedef struct {
  word_t gpr[32];
  vaddr_t pc;
} CPU_state;

#endif
