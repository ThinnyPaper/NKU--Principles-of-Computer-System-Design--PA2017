#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

#define MAX_LEN 128

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char* expr;
  uint32_t value;

} WP;

WP* new_wp();
void free_wp(int no);
bool check_wp();
void show_wp();
WP* get_head();
WP* get_free();
#endif