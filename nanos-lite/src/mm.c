#include "proc.h"
#include "memory.h"

static void *pf = NULL;

void* new_page(void) {
  assert(pf < (void *)_heap.end);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uint32_t new_brk) {
  if(current->cur_brk == 0){
	  current->cur_brk = current->max_brk = new_brk;
  }else{
	  if(new_brk>current->max_brk){
	    //TODO:map memory region [current->max_brk,new_brk]
	    //into address space current->as
	    // it's similar with loader
	    int new_space = new_brk-PGROUNDUP(current->max_brk);
      //int new_space = new_brk-current->max_brk;

      //cause max_brk may in the middle of page;
	    void *cur_vaddr = (void*)PGROUNDUP(current->max_brk);
	    void *cur_paddr;
	    while(new_space>0){
        cur_paddr=new_page();
	      _map(&current->as,cur_vaddr,cur_paddr);	
        cur_vaddr+=PGSIZE;
        new_space-=PGSIZE;
  	  }
	    current->max_brk = new_brk;
	  }
	  current->cur_brk = new_brk;
  }
  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _pte_init(new_page, free_page);
}
