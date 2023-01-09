/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "sdb.h"
#include <watchpoint.h>
#include <cpu/cpu.h>
#include <isa.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>
#include <readline/history.h>
#include <readline/readline.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();


/* We use the `readline' library to provide more flexibility to read from stdin.
 */
static char* rl_gets() {
    static char* line_read = NULL;

    if (line_read) {
        free(line_read);
        line_read = NULL;
    }

    line_read = readline("(nemu) ");

    if (line_read && *line_read) {
        add_history(line_read);
    }

    return line_read;
}

static int cmd_c(char* args) {
    cpu_exec(-1);
    return 0;
}

static int cmd_q(char* args) {
    nemu_state.state = NEMU_QUIT;
    return -1;
}

static int cmd_si(char* args) {
    char* arg = strtok(NULL, " ");
    int steps;

    if (arg == NULL) {
        cpu_exec(1);
        return 0;
    }
    sscanf(arg, "%d", &steps);
    if (steps < -1) {
        Error("error: Step(s) must be greater than or equal to -1");
        return 0;
    }
    // Assert(steps >= -1, "error: Step(s) must be greater than or equal to -1"
    // );
    cpu_exec(steps);
    return 0;
}

static int cmd_info(char* args) {
    char* arg = strtok(NULL, " ");
    if (arg == NULL) {
        Error("error: Missing parameter r or w");
        return 0;
    }
    if (strcmp(arg, "r") == 0) {
        isa_reg_display();
    }
    if (strcmp(arg, "w") == 0) {
        print_wp();
    }
    return 0;
}
static int cmd_x(char* args) {
    char* N = strtok(NULL, " ");
    char* EXPR = strtok(NULL, "");
    int num;
    bool tmp = true;
    vaddr_t addr;
    if (N == NULL || EXPR == NULL) {
        Error("error: Need two parameters N and EXPR");
        return 0;
    }
    sscanf(N, "%d", &num);
    // sscanf(EXPR, "%lx", &addr);
    addr = expr(EXPR, &tmp);
    for (int i = 0; i < num; i++) {
        word_t data = vaddr_read(addr + i * 4, 4);
        printf("addr:" FMT_WORD, addr + i * 4);
        printf("\tdata: 0x%08lx", data);
        // for (int j = 0; j < 4; j++) {
        //     printf("\t0x%02lx", vaddr_read(addr + i * 4 + 3 - j, 1));
        // }
        printf("\n");
    }
    return 0;
}

static int cmd_p(char* args) {
    bool tmp = false;
    uint64_t ans = expr(args, &tmp);
    if (tmp) {
        Print(ANSI_FG_GREEN, "Successfully evaluate the EXPR!");
        Print(ANSI_FG_MAGENTA, "EXPR:");
        printf("%s\n", args);
        Print(ANSI_FG_MAGENTA, "ANSWER:");
        printf("[Dec] unsigned: %lu signed: %ld\n[Hex] " FMT_WORD "\n", ans,
               ans, ans);
    } else
        Print(ANSI_FG_RED, "EXPR is illegal!");
    return 0;
}

static int cmd_w(char* args) {
    WP* wp = new_wp(args);
    printf("watchpoint %d: %s is set!\n", wp->NO, wp->e);
    return 0;
}

static int cmd_d(char* args) {
    if (args == NULL) {
        Error("error: Missing parameter N");
        return 0;
    }
    int N;
    bool search = true;
    sscanf(args, "%d", &N);
    WP* wp = delete_wp(N, &search);
    if (search) {
        printf("Delete watchpoint %d: %s\n", wp->NO, wp->e);
        free_wp(wp);
        return 0;
    } else {
        Print(ANSI_FG_RED, "Can't find watchpoint %d", N);
        return 0;
    }
    return 0;
}

static int cmd_help(char* args);

static struct {
    const char* name;
    const char* description;
    int (*handler)(char*);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "Step over, default N=1", cmd_si},
    {"info", "info r: print register info; info w: print watchpoint info",
     cmd_info},
    {"x", "x N EXPR: print N * 4 bytes info in mem from addr=EXPR", cmd_x},
    {"p", "p EXPR: evaluate the expression", cmd_p},
    {"w", "w EXPR: set a watchpoint on the value of EXPR", cmd_w},
    {"d", "d N: delete the watchpoint with N", cmd_d},

    /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char* args) {
    /* extract the first argument */
    char* arg = strtok(NULL, " ");
    int i;

    if (arg == NULL) {
        /* no argument given */
        for (i = 0; i < NR_CMD; i++) {
            printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        }
    } else {
        for (i = 0; i < NR_CMD; i++) {
            if (strcmp(arg, cmd_table[i].name) == 0) {
                printf("%s - %s\n", cmd_table[i].name,
                       cmd_table[i].description);
                return 0;
            }
        }
        printf("Unknown command '%s'\n", arg);
    }
    return 0;
}

void sdb_set_batch_mode() {
    is_batch_mode = true;
}

void sdb_mainloop() {
    if (is_batch_mode) {
        cmd_c(NULL);
        return;
    }

    for (char* str; (str = rl_gets()) != NULL;) {
        char* str_end = str + strlen(str);

        /* extract the first token as the command */
        char* cmd = strtok(str, " ");
        if (cmd == NULL) {
            continue;
        }

        /* treat the remaining string as the arguments,
         * which may need further parsing
         */
        char* args = cmd + strlen(cmd) + 1;
        if (args >= str_end) {
            args = NULL;
        }

#ifdef CONFIG_DEVICE
        extern void sdl_clear_event_queue();
        sdl_clear_event_queue();
#endif

        int i;
        for (i = 0; i < NR_CMD; i++) {
            if (strcmp(cmd, cmd_table[i].name) == 0) {
                if (cmd_table[i].handler(args) < 0) {
                    return;
                }
                break;
            }
        }

        if (i == NR_CMD) {
            printf("Unknown command '%s'\n", cmd);
        }
    }
}

void init_sdb() {
    /* Compile the regular expressions. */
    init_regex();

    /* Initialize the watchpoint pool. */
    init_wp_pool();
}
