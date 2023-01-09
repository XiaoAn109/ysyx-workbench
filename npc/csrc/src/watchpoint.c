#include "../include/watchpoint.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char *e) {
  assert(free_ != NULL);
  WP *tmp = free_;
  free_ = free_->next;
  tmp->next = NULL;

  bool success = false;
  strcpy(tmp->e, e);
  tmp->value = expr(tmp->e, &success);
  assert(success);

  if(head == NULL)
    head = tmp;
  else {
    WP *p = head;
    while(p->next) p = p->next;
    p->next = tmp;
  } //tail insert
  return tmp;
}


void free_wp(WP *wp) {
  assert(head != NULL);
  assert(wp != NULL);
  if(wp == head) head = head->next;
  else {
    WP *tmp = head;
    while(tmp != NULL && tmp->next != wp) tmp = tmp->next;
    tmp->next = wp->next;
  }
  wp->next = free_;
  free_ = wp;
  wp->value = 0;
  wp->e[0] = '\0';
}

bool check_wp() {
  bool check = false;
  bool _success = false;
  WP *tmp = head;
  uint64_t ans, pc;
  while(tmp != NULL) {
    ans = expr(tmp->e, &_success);
    if(ans != tmp->value) {
      check = true;
      pc = expr((char *)"$pc", &_success);
      Print(ANSI_FG_RED, "Hit watchpoint %d at address " FMT_WORD, tmp->NO, pc);
      printf("Watchpoint %d: %s\n", tmp->NO, tmp->e);
      printf("Old value = " FMT_WORD "\n", tmp->value);
      printf("New value = " FMT_WORD "\n", ans);
    }
    tmp->value = ans;
    tmp = tmp->next;
  }
  return check;
}

void print_wp() {
  WP *tmp = head;
  if(tmp == NULL) {
    Print(ANSI_FG_RED, "No watchpoints!");
  }
  while(tmp != NULL) {
    printf("Watch point %d: %s value: " FMT_WORD "\n", tmp->NO, tmp->e, tmp->value);
    tmp = tmp->next;
  }
}

WP* delete_wp(int N, bool *search) {
  WP *tmp = head;
  while(tmp != NULL && tmp->NO != N) {
    tmp = tmp->next;
  }
  if(tmp == NULL) {
    *search = false;
  }
  return tmp;
}

