#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) { 
  rtl_sext(&id_dest->val,&id_dest->val,id_dest->width);
  rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(pop) {
  //we can't use rtl_pop(&id_dest->val) directly
  rtl_pop(&t2);
  operand_write(id_dest,&t2);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  //push gnr sequentially
  if (decoding.is_operand_size_16) {
 	t0=(uint32_t)cpu.gpr[R_SP]._16;
  	rtl_push((uint32_t*)&cpu.gpr[R_AX]._16);
  	rtl_push((uint32_t*)&cpu.gpr[R_CX]._16);
  	rtl_push((uint32_t*)&cpu.gpr[R_DX]._16);
 	rtl_push((uint32_t*)&cpu.gpr[R_BX]._16);
  	rtl_push(&t0);
 	rtl_push((uint32_t*)&cpu.gpr[R_BP]._16);
  	rtl_push((uint32_t*)&cpu.gpr[R_SI]._16);
  	rtl_push((uint32_t*)&cpu.gpr[R_DI]._16);
  }
  else {
 	t0=cpu.esp;
  	rtl_push(&cpu.eax);
  	rtl_push(&cpu.ecx);
  	rtl_push(&cpu.edx);
 	rtl_push(&cpu.ebx);
  	rtl_push(&t0);
 	rtl_push(&cpu.ebp);
  	rtl_push(&cpu.esi);
  	rtl_push(&cpu.edi);
  }
  print_asm("pusha");
}

make_EHelper(popa) {
  if (decoding.is_operand_size_16) {
  	rtl_pop((uint32_t*)&cpu.gpr[R_DI]._16);
  	rtl_pop((uint32_t*)&cpu.gpr[R_SI]._16);
 	rtl_pop((uint32_t*)&cpu.gpr[R_BP]._16);
  	rtl_pop((uint32_t*)&t0);
 	rtl_pop((uint32_t*)&cpu.gpr[R_BX]._16);
  	rtl_pop((uint32_t*)&cpu.gpr[R_DX]._16);
  	rtl_pop((uint32_t*)&cpu.gpr[R_CX]._16);
  	rtl_pop((uint32_t*)&cpu.gpr[R_AX]._16);
  }
  else {
 	rtl_pop(&cpu.edi);
  	rtl_pop(&cpu.esi);
 	rtl_pop(&cpu.ebp);
  	rtl_pop(&t0);
 	rtl_pop(&cpu.ebx);
  	rtl_pop(&cpu.edx);
  	rtl_pop(&cpu.ecx);
  	rtl_pop(&cpu.eax);
       //needn't give to to esp.
  }

  print_asm("popa");
}

make_EHelper(leave) {
//it equals to mov+pop
  rtl_mv(&cpu.esp, &cpu.ebp);
  rtl_pop(&cpu.ebp);
  //operand_write(id_dest,&cpu.ebp);
  print_asm("leave");
}

make_EHelper(cltd) {
  //double the size of the source operand (EAX) through out EDX
  //only operate data on EAX
  if (decoding.is_operand_size_16) {
    rtl_lr(&t2,R_AX,2);
    if((int32_t)(int16_t)(t2)<0){//double the size of signal
	rtl_addi(&t2,&tzero,0xffff);
	rtl_sr(R_DX,2,&t2);
    }else{
	rtl_sr(R_DX,2,&tzero);
    }
  }
  else {//size_32
    rtl_lr(&t2,R_EAX,4);
    if((int32_t)t2<0){
	rtl_addi(&t2,&tzero,0xffffffff);
	rtl_sr(R_EDX,4,&t2);
    }else{
	rtl_sr(R_EDX,4,&tzero);
    }
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  //AT&T. In Intel is names cwde
  //double the size of EAX by using signal extension
  //if operand_size is 16, extends AL to AX
  //if operand_size is 32, extends AX to EAX
  if (decoding.is_operand_size_16) {
    rtl_lr(&t0,R_AL,1);
    //we can't use sext cause it extends to 32bit.
    t0 = (int16_t)(int8_t)t0;
    rtl_sr(R_AX,2,&t0);
  }
  else {
    rtl_lr(&t0,R_AX,2);
    //too lazy to use sext,put it outside.
    t0 = (int32_t)(int16_t)t0;
    rtl_sr(R_EAX,4,&t0);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
