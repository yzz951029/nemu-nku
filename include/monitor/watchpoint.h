#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  uint32_t val;
  struct watchpoint *next;
  char expr[32];
  int b;

  /* TODO: Add more members if necessary */


} WP;


WP* new_wp ();
void free_wp(WP *);
bool check_wp();
void delete_wp(int );
void info_wp();
bool check_wp();

#endif
