#ifndef __SDB_H__
#define __SDB_H__

#include "common.h"
#include <readline/readline.h>
#include <readline/history.h>
#include "memory.h"
#include "basic_func.h"
char* rl_gets();
int cmd_c(char* args);
int cmd_q(char* args);
int cmd_si(char* args);
int cmd_info(char* args);
int cmd_x(char* args);
int cmd_p(char* args);
int cmd_w(char* args);
int cmd_d(char* args);
int cmd_help(char* args);
int cmd_help(char* args);
void sdb_set_batch_mode();
void sdb_mainloop();


#endif
