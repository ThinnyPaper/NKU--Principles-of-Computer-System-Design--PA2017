#include "common.h"
#include "syscall.h"

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4], ret_value=0;;
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none:
      ret_value=1;
      break;
    case SYS_exit:
      _halt(a[1]);
      break;
    case SYS_write:{
      Log("in write");
      
      break;
    }
    
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  SYSCALL_ARG1(r)=ret_value;
  return NULL;
}
