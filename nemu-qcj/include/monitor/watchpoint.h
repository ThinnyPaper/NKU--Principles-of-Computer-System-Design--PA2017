#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char* expr;
  uint32_t value;
  bool isfree;
} WP;
WP* newwp(char * args,uint32_t value);
bool free_wp(int n);
void print_wp();
WP* get_head();
WP* get_free();
#endif
