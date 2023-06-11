#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  unsigned long long mul = a * b;
  return mul>>16;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  assert(b!=0);
  unsigned long long div = a<<16;
  div = div/b;
  return div;
}

/*float occupies 32bit to store a number.
  32th bit: sign
  24-31th bits : exp
  1:23 bits: tail-number 
*/

struct FloatBit{
  uint32_t num :23;
  uint32_t exp :8;
  uint32_t sign:1;
};

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */
  struct FloatBit *float_bit = (struct FloatBit*)&a;
  int exp = float_bit->exp - 127;
  unsigned int num = float_bit->num & 0x7fffff;

  if(float_bit->exp==0){
  	exp++;
  }else{
	num |= (1<<23); //pre-bit
  }

  unsigned int result;
  //now the point is between 23-24bit,but FLOAT's point is between 16-17 bit.
  //So we need right shift 7bit from float to FLOAT.
  //BUT it will lost some num.We should consider the exp to deside whether we need to right shift.
  if(exp<7){
  	result = num>>(7-exp);
  }else if(exp>=7 && exp<22){
	result = num<<(exp-7);
  }else{
     //overflow
     assert(0);
  }
  
  return result;
}

FLOAT Fabs(FLOAT a) {
  return (a>=0)?a:-a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}
