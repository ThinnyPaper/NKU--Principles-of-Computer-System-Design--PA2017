#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //1.save state
  rtl_push(&cpu.EFLAGS.all_flags);
  rtl_push(&cpu.CS);
  rtl_push(&ret_addr);//don't use cpu.eip. it's out of date.

  //2&3.take IDTR base-addr and get gate dicriptor
  //each gate dicriptor has 8 bytes.
  uint32_t gate_low=vaddr_read(cpu.IDTR.base+NO*8,4);
  uint32_t gate_high=vaddr_read(cpu.IDTR.base+NO*8+4,4);
  
  //4.extract offset and combine to 32-bits offset
  uint32_t offset_low=gate_low&0x0000ffff;
  uint32_t offset_high=gate_high&0xffff0000;
  uint32_t offset=offset_high|offset_low;
  
  //5.goto target address
  decoding.jmp_eip=offset;
  decoding.is_jmp=true;
}

void dev_raise_intr() {
}
