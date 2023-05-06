#include "proc.h"

#define MAX_NR_PROC 4
#define CHANGE_NUM 1000

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;
int cnt=0;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  int i = nr_proc ++;
  _protect(&pcb[i].as);

  uintptr_t entry = loader(&pcb[i].as, filename);

  // TODO: remove the following three lines after you have implemented _umake()
//  _switch(&pcb[i].as);
//  current = &pcb[i];
//  ((void (*)(void))entry)();

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
}

_RegSet* schedule(_RegSet *prev) {
  current->tf = prev;

  if(cnt++==CHANGE_NUM){
  	current = &pcb[1];
 	  cnt=0;
  }else{
  	current = &pcb[0];
  }

  _switch(&current->as);

  //Log("0x%x  0x%x",current->tf,current->as);
  return current->tf;
  }
