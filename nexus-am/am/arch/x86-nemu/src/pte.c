#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

void _map(_Protect *p, void *va, void *pa) {
  PDE *pde=(PDE*)(p->ptr);
  off_t pde_offset = PDX(va);
  // if no page table, alloc one.
  if(!(pde[pde_offset] & PTE_P)){
    PTE* new_pte=(PTE*)(palloc_f());
    pde[pde_offset]=( (uint32_t)(new_pte) | PTE_P);
  }

  PTE* pte_base_addr=(PTE*)(pde[pde_offset] & 0xFFFFF000);
  off_t pte_offset = PTX(va);
  pte_base_addr[pte_offset]=((uint32_t)pa | PTE_P);
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {

  uint32_t *sp = ustack.end;
  //_start reg init  
  
  for(int i=0;i<8;i++,sp--)
      *sp = 0;
  
  // *sp=0x2;sp--;  //init eflags
  *sp=0x202;sp--; //init eflags
  *sp=0x8;sp--;  //init cs
  *sp=(uint32_t)entry;sp--; //init eip
  *sp=0;sp--; //init errorcode
  *sp=0x81;sp--; //init irq
  
  //init reg
  for(int i=0;i<7;i++,sp--)
    *sp=0;
  *sp=0;
  return (_RegSet*)sp;
}
