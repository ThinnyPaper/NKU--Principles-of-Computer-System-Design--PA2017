#include "common.h"
#define DEFAULT_ENTRY ((void *)0x8048000)
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_filesz(int fd);
extern int fs_close(int fd);

extern void* new_page(void);

uintptr_t loader(_Protect *as, const char *filename) {
  //ramdisk_read(DEFAULT_ENTRY,0,get_ramdisk_size());
  int fd=fs_open(filename,0,0);
  int filesize=fs_filesz(fd);
  void* page_vaddr=DEFAULT_ENTRY, *page_paddr;
  while(filesize>0){
    page_paddr=new_page();
    _map(as,page_vaddr,page_paddr);	
    fs_read(fd,page_paddr,PGSIZE);
    page_vaddr+=PGSIZE;
    filesize-=PGSIZE;
  }
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}

