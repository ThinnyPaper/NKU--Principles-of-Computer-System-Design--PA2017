#include "common.h"
#include "syscall.h"
//PA3
extern ssize_t fs_write(int fd, const void *buf, size_t len);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern off_t fs_lseek(int fd, off_t offset, int whence);
extern int fs_open(const char *pathname, int flags, int mode);
extern int fs_close(int fd);
_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4],ret_value=0;
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
    case SYS_write:
      //Log("in the write\n");
      ret_value=fs_write(a[1],(void*)a[2], a[3]);
      break;
    case SYS_read:
      //Log("in the read\n");
      ret_value=fs_read(a[1],(void*)a[2],a[3]);
      break;
    case SYS_brk:
      ret_value=0;
      break;
        case SYS_close:
      ret_value=fs_close(a[1]);
      break;
    case SYS_lseek:
      Log("in the lseek\n");
      ret_value=fs_lseek(a[1],a[2],a[3]);
      break;
    case SYS_open:
      ret_value=fs_open((char*)a[1],a[2],a[3]);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  SYSCALL_ARG1(r)=ret_value;
  return NULL;
}
