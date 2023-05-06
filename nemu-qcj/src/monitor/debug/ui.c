#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
  if(args==NULL){ 
    printf("Si [NUM] must has a number\n");
    return 0;
  }
  int n=0;
  sscanf(args,"%d\n",&n);
  cpu_exec(n);
  return 0;
}
static int cmd_info(char *args) {
  if(args==NULL){
    printf("Use info r OR info w\n");
    return 0;
  }
  char ch;
  sscanf(args,"%c\n",&ch);
  switch(ch){
    case 'r':{
	for(int i=0;i<8;i++){
	    printf("%s: 0x%08X\n",regsl[i],cpu.gpr[i]._32);
	}
	printf("eip: 0x%08X\n",cpu.eip);
	printf("CF: %d\n",cpu.EFLAGS.CF);
	printf("ZF: %d\n",cpu.EFLAGS.ZF);
	printf("OF: %d\n",cpu.EFLAGS.OF);
	printf("SF: %d\n",cpu.EFLAGS.SF);
	printf("IF: %d\n",cpu.EFLAGS.IF);

	break;
    }
    case 'w':{
	print_wp();
	break;
    }
    default:{
	printf("Please Use info r OR info w\n");
	break;
    }
  }
  return 0;
}

static int cmd_x(char* args){
    if(args==NULL){
	printf("Please Use x N EXPR Format.\n");
	return 0;
    }

    int n;//n*4byte to scan
    vaddr_t start_addr;// start address of scanning.
    sscanf(strtok(args," "),"%d",&n);
    
    //printf("%s",args);
    printf("%d\n",n);
    char * token=strtok(NULL," ");//the next token which contains start_addr
    if(token==NULL){
	printf("Please Use x N EXPR Format.\n");
	return 0;
    }
    sscanf(token,"0x%X",&start_addr);
    printf("0x%08X\n",start_addr);
    for(int i=0;i<n;i++){
	uint32_t data=vaddr_read(start_addr,4);
	printf("0x%08X: 0x%08X\n",start_addr,data);
	start_addr+=4;
    }
    return 0;
}

static int cmd_p(char* args){
    bool success=true;
    uint32_t ans=expr(args,&success);
    if(!success){
	printf("expr is wrong,please check\n");
	return 0;
    }
    printf("%s = %d\n",args,ans);
    return 0;
}

static int cmd_w(char* args){
	bool success=true;
	uint32_t ans=expr(args,&success);
	if(!success){
		printf("expr is wrong,please check\n");
		return 0;
	}
	WP* wp=newwp(args,ans);
	printf("Set watchpoint No.%d. %s:%u\n",wp->NO,wp->expr,wp->value);
	return 0;
}
static int cmd_d(char* args){
	uint32_t i=s2i(args);
	if(free_wp(i))printf("watchpoint No.%d is free.\n",i);
	return 0;
}
static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  /* TODO: Add more commands */
  { "si", "Exec by steps", cmd_si },
  { "info", "Print register/watchPoint information", cmd_info},
  { "x", "Scan Memory", cmd_x},
  { "p", "Calculate Expression",cmd_p},
  { "w", "Set Watchpoint",cmd_w},
  { "d", "Delete Watchpoint",cmd_d},
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }
    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
