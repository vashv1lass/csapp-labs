/* 
 * CS:APP Data Lab 
 * 
 * Max @vashv1las Vashkevich
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

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
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
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


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

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  int   xory = ~(~x & ~y);
  int xnandy = ~(x & y);

  return xory & xnandy;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 1 << 31;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  int    ones = ~0;
  int xmul2p1 = x + x + 1;

  return !(ones ^ xmul2p1) & !!(x ^ ones);
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int allodd_mask = (0xAA << 24) | (0xAA << 16) | (0xAA << 8) | 0xAA;
  int     xallodd = allodd_mask & x;
  int  negxallodd = ~xallodd + 1;

  return !(allodd_mask + negxallodd);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  int nolonibble_fits = !((x >> 4) ^ 0x3);
  int   lonibble_fits = !(x & 0x8) | !((x & 0xF) ^ 0x8) | !((x & 0xF) ^ 0x9);

  return nolonibble_fits & lonibble_fits;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  int muxx = (!!x) << 31 >> 31;

  return (muxx & y) | (~muxx & z);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int tmin = 1 << 31;

  int   negx = ~x + 1;
  int ydiffx = negx + y;

  int eqsign_mask = (negx ^ y) >> 31;
  int   yneg_mask = y >> 31;

  int xtmin_mask = (!(x ^ tmin) << 31) >> 31;

  int result = (eqsign_mask & !(ydiffx >> 31)) | (~eqsign_mask & !(yneg_mask));
  result = (xtmin_mask & 1) | (~xtmin_mask & result);

  return result;
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  int  spread1 = (x >> 1) | x;
  int  spread2 = (spread1 >> 2) | spread1;
  int  spread4 = (spread2 >> 4) | spread2;
  int  spread8 = (spread4 >> 8) | spread4;
  int spread16 = (spread8 >> 16) | spread8;

  return ~spread16 & 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
	int  negx_mask = (x & (1 << 31)) >> 31;
	int       absx = (negx_mask & (~x + 1)) | (~negx_mask & x);
	int  save_absx = absx;
	int zerox_mask = (!(x ^ 0) << 31) >> 31; // the 'x=0' edge case
	
	int        first1 = 0;
	int   curshiftval = 0;
	int curshift_mask = 0;

  int incr_mask = 0;
  int   incrval = 0;

	curshift_mask = (!!(absx >> 16) << 31) >> 31;
	curshiftval = (curshift_mask & 16) | (~curshift_mask & 0);
	absx >>= curshiftval;
	first1 |= curshiftval;
	
	curshift_mask = (!!(absx >> 8) << 31) >> 31;
	curshiftval = (curshift_mask & 8) | (~curshift_mask & 0);
	absx >>= curshiftval;
	first1 |= curshiftval;
	
	curshift_mask = (!!(absx >> 4) << 31) >> 31;
	curshiftval = (curshift_mask & 4) | (~curshift_mask & 0);
	absx >>= curshiftval;
	first1 |= curshiftval;
	
	curshift_mask = (!!(absx >> 2) << 31) >> 31;
	curshiftval = (curshift_mask & 2) | (~curshift_mask & 0);
	absx >>= curshiftval;
	first1 |= curshiftval;
	
	curshift_mask = (!!(absx >> 1) << 31) >> 31;
	curshiftval = (curshift_mask & 1) | (~curshift_mask & 0);
	absx >>= curshiftval;
	first1 |= curshiftval;
	
	incr_mask = (!(save_absx ^ (1 << first1)) << 31) >> 31;
	incr_mask = (negx_mask & incr_mask) | (~negx_mask & 0);
	incr_mask = (zerox_mask & ~0) | (~zerox_mask & incr_mask);
	
	incrval = !incr_mask;

	return first1 + 1 + incrval;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
#if 0 // Replace 0 to 1 to compile a CHALLENGE
  // Integer coding rules restriction CHALLENGE (Max ops: 40) (no constant limits):

  unsigned ufsign_mask =  uf & 0x80000000;
  unsigned       ufexp = (uf & 0x7F800000) >> 23;
  unsigned      uffrac =  uf & 0x007FFFFF;

  int        ufspec_mask = !(ufexp ^ 0xFF) << 31;
  int      ufdenorm_mask = !ufexp << 31;
  int ufpreoverflow_mask = !(ufexp ^ 0xFE) << 31;

  int uffrac_res = uffrac;

  ufspec_mask >>= 31;
  ufdenorm_mask >>= 31;
  ufpreoverflow_mask >>= 31;

  uffrac_res = (ufdenorm_mask & (uffrac_res << 1)) | (~ufdenorm_mask & uffrac_res);
  uffrac_res = (ufpreoverflow_mask & 0) | (~ufpreoverflow_mask & uffrac_res);
  uffrac_res = (ufspec_mask & uffrac) | (~ufspec_mask & uffrac_res);

  ufexp = (ufdenorm_mask & ufexp) | (~ufdenorm_mask & (ufexp + 1));
  ufexp = (ufspec_mask & 0xFF) | (~ufspec_mask & ufexp);

  return ufsign_mask | ((ufexp << 23) + uffrac_res);
#else
  unsigned ufsign_mask =  uf & 0x80000000;
	unsigned       ufexp = (uf & 0x7F800000) >> 23;
	unsigned      uffrac =  uf & 0x007FFFFF;
	
	if (ufexp == 0xFF) { // case 1: the number is special
		return uf;
	}
	
	if (ufexp == 0x00) { // case 2: the number is denormalized
		uffrac <<= 1;
		
		return ufsign_mask | uffrac;
	}
	
	// case 3: the number is normalized
	++ufexp;
	
	if (ufexp == 0xFF) { // case 3.1: the number has overflowed
		return ufsign_mask | 0x7F800000;
	}
	
	// case 3.2: a usual case
	return ufsign_mask | (ufexp << 23) | uffrac;
#endif // Integer coding rules restriction CHALLENGE
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
#if 0 // Replace 0 to 1 to compile a CHALLENGE
  // Integer coding rules restriction CHALLENGE (Max ops: 40) (does not work correctly)

  int      ufsign_mask = (uf & 0x80000000) >> 31;
  unsigned       ufexp = (uf & 0x7F800000) >> 23;
  unsigned      uffrac =  uf & 0x007FFFFF;

  unsigned          ufnormsignificand = uffrac | 0x00800000;
  int      ufnormsignificand_shiftval = 150 - ufexp;
  int      ufnormsignificandsign_mask = ufnormsignificand_shiftval >> 31;

  unsigned negufexp = ~ufexp + 1;
  int   uf2big = !((127 + negufexp) & 0x80000000) << 31 >> 31;
  int uf2small = ((157 + negufexp) & 0x80000000) >> 31;

  ufnormsignificand = (ufnormsignificandsign_mask & (ufnormsignificand << (~ufnormsignificand_shiftval + 1))) | (~ufnormsignificandsign_mask & (ufnormsignificand >> ufnormsignificand_shiftval));
  ufnormsignificand = (ufsign_mask & (~ufnormsignificand + 1)) | (~ufsign_mask & ufnormsignificand);
  ufnormsignificand = (uf2big & 0x80000000) | (~uf2big & ufnormsignificand);
  ufnormsignificand = (uf2small & 0) | (~uf2small & ufnormsignificand);

  return ufnormsignificand;
#else
	unsigned ufsign_mask =  uf & 0x80000000;
	unsigned       ufexp = (uf & 0x7F800000) >> 23;
	unsigned      uffrac =  uf & 0x007FFFFF;
	
	unsigned          ufnormsignificand = uffrac | 0x00800000;
	int      ufnormsignificand_shiftval = 150 - ufexp;
	
	if (ufexp < 127) { // case 1: the number is too small
		return 0;
	}
	
	if (ufexp >= 158) { // case 2: the number is too big, special, or 0xCF000000
		return 0x80000000;
	}
	
	// case 3: the number is normalized
	if (ufnormsignificand_shiftval > 0) {
		ufnormsignificand >>= ufnormsignificand_shiftval;
	} else {
		ufnormsignificand <<= ufnormsignificand_shiftval;
	}
	
	if (ufsign_mask) {
		ufnormsignificand = -ufnormsignificand;
	}
	
	return ufnormsignificand;
#endif
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  return 2;
}
