#include "cpu/exec.h"

//Data Movement
make_EHelper(mov);
make_EHelper(push);
make_EHelper(pop);

//arith
make_EHelper(sub);
make_EHelper(sbb);

//Logical
make_EHelper(xor);

//Control
make_EHelper(jmp);
make_EHelper(call);
make_EHelper(call_rm);
make_EHelper(ret);

//Miscellaneous
make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);


