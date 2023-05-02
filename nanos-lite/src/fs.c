#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);

extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern void fb_write(const void *buf, off_t offset, size_t len);
extern size_t events_read(void *buf, size_t len);

int fs_open(const char *pathname, int flags, int mode);
ssize_t fs_read(int fd, void *buf, size_t len);
ssize_t fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);
size_t fs_filesz(int fd);
/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

int fs_open(const char *pathname, int flags, int mode){
  for(int i=0;i<NR_FILES;i++){
	if(strcmp(file_table[i].name,pathname)==0){
		return i;
	}
  }
  Log("cannot open the file %s\n",pathname);
  assert(0);
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len){
    size_t filesize=fs_filesz(fd);
   // printf("fd:%d open_offset: %d\n",fd,file_table[fd].open_offset);
    switch(fd){
	  case FD_DISPINFO:
	    if(file_table[fd].open_offset<filesize&&len!=0){
		    len=len<(filesize-file_table[fd].open_offset)?len:filesize-file_table[fd].open_offset;
	    	dispinfo_read(buf,file_table[fd].open_offset,len);
		    file_table[fd].open_offset+=len;
		    return len;
	    }
	    break;
	  case FD_EVENTS:
	    return events_read(buf,len);
    default:
	    //take minimum between len and the rest len of file.
	    if(file_table[fd].open_offset<filesize&&len!=0){
	    	len=len<(filesize-file_table[fd].open_offset)?len:filesize-file_table[fd].open_offset;
		    ramdisk_read(buf,file_table[fd].disk_offset+file_table[fd].open_offset,len);
	      file_table[fd].open_offset+=len;//update offset
	       return len;   	    
	    }
	    
    }
    return 0;
}

ssize_t fs_write(int fd, const void *buf, size_t len){
	//Log("in write");
	size_t filesize=fs_filesz(fd);
	switch(fd){
	  case FD_STDOUT:
	  case FD_STDERR:
	    for(int i=0;i<len;i++){
	      _putc(((char*)buf)[i]);
	    }
	    return len;
	  case FD_FB:
	    //Log("in fb");
	    if(len!=0){
        fb_write(buf,file_table[fd].open_offset,len);
        file_table[fd].open_offset+=len;
        return len;
	    }
	    break;
	  default:
	    if(file_table[fd].open_offset<filesize&&len!=0){
	    	len=len<(filesize-file_table[fd].open_offset)?len:filesize-file_table[fd].open_offset;
		    ramdisk_write(buf,file_table[fd].disk_offset+file_table[fd].open_offset,len);
	      file_table[fd].open_offset+=len;//update offset
	      return len;   	    
	    }
	    break;
	}
	return 0;
}
off_t fs_lseek(int fd, off_t offset, int whence){
/*
    whence: where from.
    it has Three Mode:
    1.SEEK_SET: set the offset given as new open_offset
    2.SEEK_CUR: add the offset given to open_offset.The offset can be negative.
    3.SEEK_END: get the end of file and add offset.The offset can be negative.
*/
   // printf("call lseek\n");
    size_t filesize=fs_filesz(fd);
    switch(whence){
	  case SEEK_SET:
	    if(offset>=0&&offset<=filesize){
		file_table[fd].open_offset=offset;
	    }else{return -1;}
	    break;
	  case SEEK_CUR:
	    if(file_table[fd].open_offset+offset>=0&&file_table[fd].open_offset+offset<=filesize){
	    	file_table[fd].open_offset+=offset;
	    }else{return -1;}
	    break;
	  case SEEK_END:
	    file_table[fd].open_offset=filesize+offset;
	    break;
    }
    return file_table[fd].open_offset;
}	
int fs_close(int fd){return 0;}
size_t fs_filesz(int fd){
	return file_table[fd].size;
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
    file_table[FD_FB].size=_screen.width*_screen.height*4;
}




