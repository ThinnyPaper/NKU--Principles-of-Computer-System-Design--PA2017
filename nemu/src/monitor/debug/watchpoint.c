#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;//head=>using wp,free=>free wp

void init_wp_pool() {
  int i;
  //arrange number for each wp in acsent.
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].value = 0;
    wp_pool[i].isfree=true;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* newwp(char* args,uint32_t value){
  printf("222:%s\n",args);
  if(free_==NULL){
	Assert(0,"No space to allocate watchpoint.\n");
	return NULL;
  }
  WP* new=free_;
  new->expr=(char*)malloc(strlen(args));
  strcpy(new->expr,args);
  new->value=value;new->isfree=false;
  free_=free_->next;
  new->next=head;
  head=new;
  return new;
}

bool free_wp(int n){
	if(n>=NR_WP){
		printf("N is out of range.\n");
		return false;
	}
	WP* wp=&wp_pool[n];
	wp->expr="";
	wp->value=0;
	wp->isfree=true;
	if(!head){
		printf("No watchpoint Occupied.\n");
		return false;
	}
	WP* temp=head,*nxt=head->next;
	if(temp->NO==wp->NO){
		head=head->next;
		temp->next=free_;
		free_=temp;
		return true;
	}
	while(nxt){
		if(nxt->NO==wp->NO){
			temp->next=nxt->next;
			nxt->next=free_;
			free_=nxt;
			return false;
		}
		temp=nxt;
		nxt=nxt->next;
	}
	printf("wachpoint No.%d is not occupied.\n",wp->NO);
	return false;
}

void print_wp(){
	if(!head){
		printf("All Watchpoint Available\n.");
		return;
	}
	for(int i=0;i<NR_WP;i++){
		if(!wp_pool[i].isfree)
			printf("watchpoint %d: %s = %u\n",wp_pool[i].NO,wp_pool[i].expr,wp_pool[i].value);
	}
	return;
}
WP* get_head(){
  return head;
}
WP* get_free(){
  return free_;
}


