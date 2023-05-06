#include "cpu/exec.h"

make_EHelper(test) {
  //the same as add  but no write-back.
  rtl_and(&t2,&id_dest->val,&id_src->val);
  //update ZF,SF,(and PF,but PA2 seems to ignore this flag.
  rtl_update_ZFSF(&t2,id_dest->width);
  //clear OF,CF
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);
  //update ZF,SF,(and PF,but PA2 seems to ignore this flag.
  rtl_update_ZFSF(&t2,id_dest->width);
  //clear OF,CF
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);
  //update ZF,SF,(and PF,but PA2 seems to ignore this flag.
  rtl_update_ZFSF(&t2,id_dest->width);
  //clear OF,CF
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);
  //update ZF,SF,(and PF,but PA2 seems to ignore this flag.
  rtl_update_ZFSF(&t2,id_dest->width);
  //clear OF,CF
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(or);
}

make_EHelper(sar) {
   //we cannot use sext cause it extends to 32bit
   if(id_dest->width==1)id_dest->val=(int8_t)id_dest->val;
   if(id_dest->width==2)id_dest->val=(int16_t)id_dest->val;
   rtl_sar(&t2,&id_dest->val,&id_src->val);
   operand_write(id_dest,&t2);
  // unnecessary to update CF and OF in NEMU   fine.
  rtl_update_ZFSF(&t2,id_dest->width);
  print_asm_template2(sar);
}

make_EHelper(shl) {
   rtl_shl(&t2,&id_dest->val,&id_src->val);
   operand_write(id_dest,&t2);
  // unnecessary to update CF and OF in NEMU
  rtl_update_ZFSF(&t2,id_dest->width);
  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtl_shr(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);
  // unnecessary to update CF and OF in NEMU
  rtl_update_ZFSF(&t2,id_dest->width);
  print_asm_template2(shr);
}
make_EHelper(rol){
  //shift left and each time put the highest bit to the lowest bit  
  //to shift n time,we should get the n hightest bits to the lowest bits. 
  rtl_shri(&t0,&id_dest->val,id_dest->width*8-id_src->val);
  rtl_shl(&t1,&id_dest->val,&id_src->val);
  //concat the result
  rtl_or(&t0,&t0,&t1);
  operand_write(id_dest,&t0);
  //print(info)
  print_asm_template2(rol);
}
make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  rtl_not(&id_dest->val);
  operand_write(id_dest,&id_dest->val);
  print_asm_template1(not);
}
