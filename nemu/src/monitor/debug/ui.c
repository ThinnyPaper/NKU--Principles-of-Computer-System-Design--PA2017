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

// PA1 function-----------------------------------------------------
// section2
static int cmd_si(char *args){
  if(args == NULL) {
		cpu_exec(1);
		return 0;
	}
	char * steps = strtok(NULL, " ");
	if(steps == NULL) {
		cpu_exec(1);
	} 
  else {
		int n = 1;
		if(sscanf(steps, "%d", &n) == 1 && n > 0) { 
      cpu_exec(n); 
    }
    else { 
      printf("Bad Argument: \e[31m%s\e[0m\n", steps); 
    }
	}
	return 0;
}

static int cmd_info(char *args){
	char *arg = strtok(NULL, " ");
	if(arg==NULL){
		printf("undefined info args\n");
		return 0;
	}
	if(strcmp(arg, "r") == 0) {
		//print reg info 
    printf("eax: 0x%08x\n", cpu.eax);
    printf("ecx: 0x%08x\n", cpu.ecx);
    printf("edx: 0x%08x\n", cpu.edx);
    printf("ebx: 0x%08x\n", cpu.ebx);
    printf("esp: 0x%08x\n", cpu.esp);
    printf("ebp: 0x%08x\n", cpu.ebp);
    printf("esi: 0x%08x\n", cpu.esi);
    printf("edi: 0x%08x\n", cpu.edi);
    printf("eip: 0x%08x\n", cpu.eip);

	} 
  else if (strcmp(arg, "w") == 0) {
		// TODO: Print Breakpoint
	} 
  else {
		printf("Bad Argument: \e[31m%s\e[0m\n",arg);
	}
	return 0;
}

static int cmd_x(char *args) {
	int n;
	char *expr;
	vaddr_t addr;
	if(args == NULL) { 
		printf("Command format: x N EXPR\n");
		return 0;
	}
	if(sscanf(args, "%d", &n) == 1) {
		expr = strtok(NULL, " ");
		expr = strtok(NULL, " ");
		printf("n = %d, expr = %s\n", n, expr);
		if(expr == NULL) {
			printf("Command format: x N EXPR\n");
			return 0;
		}
    // TODO: Calculate expr

		if(sscanf(expr, "0x%08x", &addr) == 1) {
			while(n > 0) {
				printf("0x%08x:\t", addr);
        printf("0x%08x ", vaddr_read(addr, 4));
				n --;
				addr += 4;
				printf("\n");
			}
		}
    else {
      printf("Bad EXPR\n");
      return 0;
    }
	} 
  else {
		printf("Command format: x N EXPR\n");
		return 0;
	}
	return 0;
}

// section2
static int cmd_p(char *args) {
  char *expr_str = strtok(args, " ");
  printf("expr = $%s$\n", expr_str);
		if(expr_str == NULL) {
			printf("NULL EXPR\n");
			return 0;
		}
  uint32_t res = 0;
  bool success = true;
  res=expr(expr_str, &success);
  if(success == true){
    printf("result :%d\n", res);
  }
  else{
    printf("Evaluate falled\n");
  }

  return 0;
}
//end of PA1 funcion------------------------------------------------

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
	
  /* TODO: Add more commands */
  { "si", "Single setp execution", cmd_si},
  { "info", "r: Print register info; w: Print watch point info", cmd_info},
  { "x", "x N EXPR: Print memory from EXPR to EXPR+N.", cmd_x},
  { "p", "evaluate a EXPR", cmd_p}
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
