#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();
extern void raise_intr(uint8_t NO, vaddr_t ret_addr) ;

make_EHelper(lidt) {
  //use vaddr_read
  if (decoding.is_operand_size_16) {
	cpu.IDTR.limit=vaddr_read(id_dest->addr,2);
	cpu.IDTR.base=vaddr_read(id_dest->addr+2,4)&(0x00FFFFFF);
  }
  else {
	cpu.IDTR.limit=vaddr_read(id_dest->addr,2);
	cpu.IDTR.base=vaddr_read(id_dest->addr+2,4);
  }

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  //set cr
  switch(id_dest->reg){
  case 0:
    cpu.CR0=id_src->val;
    break;
  case 3:
    cpu.CR3=id_src->val;
    break;
  }
  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  //get cr
  switch(id_dest->reg){
  case 0:
	  id_dest->val=cpu.CR0;
	  break;
  case 3:
	  id_dest->val=cpu.CR3;
	  break;
  }
  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  raise_intr(id_dest->val,decoding.seq_eip);
  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  rtl_pop(&decoding.jmp_eip); //get return address
  decoding.is_jmp=1;
  rtl_pop(&cpu.CS);
  rtl_pop(&cpu.EFLAGS.all_flags);

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  t2=pio_read(id_src->val,id_src->width);
  operand_write(id_dest,&t2);

  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  pio_write(id_dest->val,id_src->width,id_src->val);

  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
