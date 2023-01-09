#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__
#include <common.h>

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char e[32];
  uint64_t value;

} WP;

WP* new_wp(char *e);
void free_wp(WP *wp);
bool check_wp();
void print_wp();
WP* delete_wp(int N, bool *search);

#endif

