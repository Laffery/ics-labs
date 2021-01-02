/* 
 * CS:APP Data Lab 
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#include "btest.h"
#include <limits.h>

/* 
 * STEP 1: Fill in the following struct with your identifying info. 
 */
team_struct team =
{
   /* Team name*/
    "518021910545", 
   /* Student name 1: Replace with the full name of first team member */
   "Ao Yuchen",
   /* Login ID 1: Replace with the login ID of first team member */
   "518021910545",
   "",""
};

#if 0
/*
 * STEP 2: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.
#endif

/*
 * STEP 3: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the btest test harness to check that your solutions produce 
 *      the correct answers. Watch out for corner cases around Tmin and Tmax.
 */
/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 9.0.0.  Version 9.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2014, fourth edition, plus
   Amd. 1  and Amd. 2 and 273 characters from forthcoming  10646, fifth edition.
   (Amd. 2 was published 2016-05-01,
   see https://www.iso.org/obp/ui/#iso:std:iso-iec:10646:ed-4:v1:amd:2:v1:en) */
/* We do not support C11 <threads.h>.  */
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  /*
   * Find that a integer which is not zero,
   * either it's highest bit is 1 and it's two's complement's highest bit is 0, 
   * or it's highest bit is 0 and it's two's complement's highest bit is 1.
   * However, zero and it's two's complment is 0x00000000 forever.
   * Thus I handle this problem by operating on x's highest bit and x's two's 
   * complement's highest bit.
   */
  int x_highest = x>>31;
  int x_twos_complement_highest = (~x+1)>>31;
  /* 
   * if x < 0, x_highest = 0xffffffff, 
   * x_twos_complement_highest = 0x00000000;
   * if x > 0, x_highest = 0x00000000, 
   * x_twos_complement_highest = 0xffffffff.
   * Thus (x_highest&1)|(x_twos_complement_highest&1) = 1.
   * if x == 0,(x_highest&1)|(x_twos_complement&1) = (0&1)|(0&1) = 0.
   * Using '~', bang(x(x!=0)) = 0x00000000, bang(0) = 0xffffffff.
   * so return like this:
   */
  
  return (~((x_highest & 1)|(x_twos_complement_highest & 1))) & 1;
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  /*
   * Our teacher had taught this problem by using mask.
   * The method is use mask 0001 to get 1 in every half byte.
   * like this:
   */
  int mask0 = (0x11<<8)+0x11;
  int mask1 = (mask0<<16)+mask0;
  int count = (x&mask1)+((x>>1)&mask1)+((x>>2)&mask1)+((x>>3)&mask1);
  /*
   * now I have used 14 ops.
   * then add counts in every byte,
   * like this:
   */
  int mask2 = 0xff+(0xff<<8);
  count = (mask2&(count>>16)) + (mask2&count);
  	/*
	 * now I have used 20ops, and
	 * count = 0x0000(count half byte 8+4)(,,,7+3)(,,,6+2)(,,,5+1);
	 * add every half byte by masks:
	 * 0x0000000f
	 * 0x000000f0
	 * 0x00000f00
	 * 0x0000f000
	 */
  count = (0xf&count) + ((0xf0&count)>>4) + 
	  (((0xf<<8)&count)>>8) + (((0xf<<12)&count)>>12);
  	/*
	 * totally use 32ops,perfect!
	 */
  return count;
}
/* 
 * copyLSB - set all bits of result to least significant bit of x
 *   Example: copyLSB(5) = 0xFFFFFFFF, copyLSB(6) = 0x00000000
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int copyLSB(int x) {
  /*
   * from the eg. 'copyLSB(5) = 0xffffffff'I get the inspiration
   * that move LSB to MSB(most), then 
   * right shift to get 0xffffffff if LSB ==  1
   */
  return (x<<31)>>31;
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
  /*
   * It's obviously that we only need to right shift n bits if
   * x >= 0.
   * While x < 0, 
   * obviously, -33/2^4 must be -(33/16) = -2 = ~2+1
   * then 15/2 = (!~)2 + 0
   * there is a special case that 0x80000000, ~0x80000000 + 1 = 0x80000000
   * this situation just need to right shift directly, thus make sign(1) to 0
   * is_special_case will be 1 when x = 0x80000000,
   * so (!is_special_case)&((x>>31)&1) will be 0 while x is special
   */
  int is_special_case = !((1<<31)^x);
  int sign = (!is_special_case)&((x>>31)&1);
  int mask = ~sign+1;
  return (mask^(((mask^x)+sign)>>n))+sign;
}
/* 
 * evenBits - return word with all even-numbered bits set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 2
 */
int evenBits(void) {
  /*
   * even-numbered bits are 1,considering 01 format, 
   * and considering max ops,
   * and considering usable maxium integer is 0x000000ff
   * we'd better use 0101010101 as a element.
   */
  int element = 0x00000055;
  return element + (element<<8) + (element<<16) + (element<<24);
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  /*
   *  if x can be represented by n bits, then bitsOf(0x(x)) + 1(sign bit) = n
   *  so just right shift n-1 bits
   *  however, '-' is illegal so I use -1 = ~1+1 to instead
   *
   *  then all bits of fits_x is sign
   *  ~sign+1 = 0xffffffff (sign=1)  or = 0x00000000 (sign=0)
   */
  int sign = (x>>31)&1;
  int n_1 = n + (~1+1);
  int fits_x = x>>n_1;
  return !(fits_x^(~sign+1));
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  /*
   * there is no doubt to use mask
   * right shift 8*n bits
   */
  n = n + n;
  n = n + n;
  n = n + n;
  return 0xff&(x>>n);
}
/* 
 * isGreater - if x > y  then return 1, else return 0 
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
  /*
   * x-y = x+(~y+1);
   * 
   * To forbid y is a very little negative so that x-y > TMAX,
   * or a very big positive so that x-y < TMIN
   * return 1 while x>=0 && y<0 directly
   * return 0 while x<0 && y>>0 directly
   * find that if sign_x is different from sign_y, 
   * the integer I should return equals sign_y,
   * so this situation just return sign_y
   *
   * other situation I can subtract safely
   *
   * from next puzzle I get that !(x>>31) can get x is nonnegative,
   * if !(x>>31) is 1, I just need to judge whether x is 0
   */
  int sign_x = (x>>31)&1;
  int sign_y = (y>>31)&1;
  int Dir_return = (sign_x^sign_y)&sign_y;
  int substract = x + (~y+1);
  int isGreater = (!(substract>>31))&(!(!substract));  
  return Dir_return | ((!(sign_x^sign_y)) & isGreater);
}
/* 
 * isNonNegative - return 1 if x >= 0, return 0 otherwise 
 *   Example: isNonNegative(-1) = 0.  isNonNegative(0) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 3
 */
int isNonNegative(int x) {
  /*
   * x >= 0, the MSB is 0
   * x <  0, the MSB is 1
   * obviously return !MSB
   */
  return !(x>>31);
}
/* 
 * isNotEqual - return 0 if x == y, and 1 otherwise 
 *   Examples: isNotEqual(5,5) = 0, isNotEqual(4,5) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int isNotEqual(int x, int y) {
  /*
   * obviously use '^',
   * if x == y, x^y = 0;
   * else !(x^y) = 0.
   */
  return !(!(x^y));
}
/*
 * isPower2 - returns 1 if x is a power of 2, and 0 otherwise
 *   Examples: isPower2(5) = 0, isPower2(8) = 1, isPower2(0) = 0
 *   Note that no negative number is a power of 2.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 60
 *   Rating: 4
 */
int isPower2(int x) {
  /*
   * if x is a power of 2, then x has only one bit 1
   * this place I can copy the puzzle 'bitCount' to judge whether x has one 1
   * however, 0x10000000 is -2^31 which isn't a power of 2
   * so &(!((x>>31)&1)) if MSB == 1, return 0
   */
  int mask0 = (0x11<<8)+0x11;
  int mask1 = (mask0<<16)+mask0;
  int mask2 = 0xff+(0xff<<8);
  int count = (x&mask1)+((x>>1)&mask1)+((x>>2)&mask1)+((x>>3)&mask1);
  count = (mask2&(count>>16)) + (mask2&count);
  count = (0xf&count) + ((0xf0&count)>>4) +
	  (((0xf<<8)&count)>>8) + (((0xf<<12)&count)>>12);

  return (!(count+(~1+1))) & (!((x>>31)&1));
}
/* 
 * leastBitPos - return a mask that marks the position of the
 *               least significant 1 bit. If x == 0, return 0
 *   Example: leastBitPos(96) = 0x20
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 4 
 */
int leastBitPos(int x) {
  /*
   * 96 = 0x60 = 1010 0000
   * -96 = 0xffffff3f +1 = 11...1 0101 1111 +1 = 11...1 0110 0000
   * more generally, if x =1.....01 0....0(n bits)
   * -x = 0.....10 1....1(n bits) +1 = 0.....11 0....0(n bits)
   * from the eg. I can find that the LBPs of x and its opporate are same
   */
  return x&(~x+1);
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 1 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  /*
   * obviously, take logicalShift(0x87654321,4) for an example,
   * right shift firstly, then use mask 0x0fffffff
   * to get the mask, just make 0xffffffff left shift 32-n bits, then ~
   */
  int arithShift = x>>n;
  int mask = ~((~0)<<(32+(~n+1)));
  return arithShift&mask;
}
/*
 * satAdd - adds two numbers but when positive overflow occurs, returns
 *          maximum possible value, and when negative overflow occurs,
 *          it returns minimum positive value.
 *   Examples: satAdd(0x40000000,0x40000000) = 0x7fffffff
 *             satAdd(0x80000000,0xffffffff) = 0x80000000
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 30
 *   Rating: 4
 */
int satAdd(int x, int y) {
  /*
   * it'll overflow while signs of x and y are same and signs of x(ory) and x+y
   * are different.
   * 
   * TMAX = 0x7fffffff + 0 = 0x7fffffff + sign_x(x>0)
   * TMIN = 0x7fffffff + 1 = 0x7fffffff + sign_x(x<0)
   */
  int sign_x = (x>>31)&1;
  int sign_y = (y>>31)&1;
  int add = x+y;
  int sign_add = (add>>31)&1;
  int TMAX = ~(1<<31);
  int isOverflow = (!(sign_x^sign_y)) & (sign_x^sign_add);
  int isNormal = (sign_x^sign_y) | ((!(sign_x^sign_y))&(!(sign_x^sign_add)));

  return ((~isOverflow+1)&(TMAX+sign_x)) | ((~isNormal+1)&add);
}
/* 
 * tc2sm - Convert from two's complement to sign-magnitude 
 *   where the MSB is the sign bit
 *   You can assume that x > TMin
 *   Example: tc2sm(-5) = 0x80000005.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 4
 */
int tc2sm(int x) {
  /*
   * if x >= 0, return x directly
   * if x < 0, return 0x80000000 + (-x)
   */
  int sign = (x>>31)&1;
  int mask = ~sign+1;

  return (mask&((1<<31) + (~x+1))) | ((~mask)&x);
}