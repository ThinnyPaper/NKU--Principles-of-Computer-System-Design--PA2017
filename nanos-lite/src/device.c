#include "common.h"
#define KEYDOWN_MASK 0x8000
#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
//read keyboard event first
  int key=_read_key();
 // printf("key: 0x%08X\n",key);
  //check up or down  
  char key_event='d';
  if(key & KEYDOWN_MASK){
      key ^= KEYDOWN_MASK;
      key_event='d';
  }else{key_event='u';}

  if(key != _KEY_NONE){
	sprintf(buf,"k%c %s\n",key_event,keyname[key]);
	return strlen(buf);
  }
  
//read time event last
  sprintf(buf,"t %d\n",_uptime());
  return strlen(buf);
 // return 0;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
	strncpy(buf,dispinfo+offset,len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
/*
    the buf is a continuous line in memory.
    but we only can draw rect.
    so image this situation:
    ---------------**************
    *****************************
    *****************************
    ************-----------------
    we need to draw 3 area:
    1. the first line
    2. center rect
    3. the end line 
*/	
    int index=offset/4, num=len/4; //num: num of pixels to write
    int start_x=index%_screen.width; //x,y: start pos
    int start_y=index/_screen.width;
    int end_x = (index+num)%_screen.width;
    int end_y = (index+num)/_screen.width;
    //only a line
    if(end_y==start_y){
	_draw_rect((uint32_t *)buf,start_x,start_y,num,1);
    }
    else if(end_y>start_y){
	//draw first line
    	_draw_rect((uint32_t *)buf,start_x,start_y,_screen.width-start_x,1);
	//if has center rect
	if(end_y-start_y>1){
	    _draw_rect((uint32_t *)buf+(_screen.width-start_x),0,start_y+1,_screen.width,end_y-start_y-1);
	}
	//draw end line
	_draw_rect((uint32_t *)buf+(num-end_x),0,end_y,end_x,1);
    }
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo,"WIDTH:%d\nHEIGHT:%d",_screen.width,_screen.height);
  printf("%s\n",dispinfo);
}
