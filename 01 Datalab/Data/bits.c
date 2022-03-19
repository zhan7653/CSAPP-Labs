/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
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
    // Easy to find x ^ y = (~x & y) | (x & ~y), so the difficulty is convert | to & and ~.
    int a = x & ~y;
    int b = ~x & y;
    return ~(~a & ~b);
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
    // tmin = 0x80000000
    return (1 << 31);

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
    // Only 0x7fffffff and 0xffffffff satisfy ~y ^ x = 0, then cut 0xffffffff out (x+1=0).
    int y = x + 1;
    return !(~y ^ x) & !!y;
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
    // Use 0xAAAAAAAA to extract all odd bits and check.
    int mask = 0xAA + (0xAA << 8) + (0xAA << 16) + (0xAA << 24);
    return !((x & mask) ^ mask);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
    // Just complement.
    return (~x + 1);
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
    // Use mask1 to check 0x30~0x37 (00110xxx), and mask2,3 to check 0x38 and 0x39.
    int mask1 = 0x3 << 4; // 0x30
    int mask2 = mask1 + 8; // 0x38
    int mask3 = mask1 + 9; // 0x39
    int checkmask1 = !((x & ~(0x07)) ^ mask1); // get 1 if x have patten 0x0000003x(0xxx)
    int checkmask2 = !(x ^ mask2); // get 1 if x = 0x38
    int checkmask3 = !(x ^ mask3); // get 1 if x = 0x39
    return checkmask1 | checkmask2 | checkmask3;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
    // Check if x = 0, then use a mask to get y or z.
    int xZero = !(x ^ 0); // get 1 if x = 0
    int mask = (~xZero) + 1; // get 0xffffffff if x = 0, get 0 otherwise.
    return (y & ~mask) | (z & mask);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
    // The result is apparent when x>=0,y<0 and x<0,y>=0, the other situations need to check y-x.
    int x_neg = ~x + 1; // get -y
    int ySubx = y + x_neg; // get y - x
    int xIsNeg = (x >> 31) & 1; // get 1 if x < 0
    int yIsNeg = (y >> 31) & 1; // get 1 if y < 0
    int yLessx = (ySubx >> 31) & 1; // get 1 if y < x
    int situation1 = (!xIsNeg) & yIsNeg; // get 1 when x >= 0, y < 0, x > y
    int situation2 = xIsNeg & (!yIsNeg); // get 1 when x < 0, y >= 0, x < y
    return situation2 | ((!situation1) & !yLessx);
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
    // if x = 0 then x ^ -x = 0 ,otherwise x ^ -x = 0x1xx...x, use the sign bit. Exclude special case Tmin.
    int x_neg = ~x + 1; // get -x
    int xNotTmin = (x >> 31) + 1; // get 1 if x is not Tmin, actually when x >= 0.
    return (((x ^ x_neg) >> 31) + 1) & xNotTmin;
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
    // Each number of bits require to represent x in 2s has a range, like 1bit : -1 ~ 0, 4bit: -8 ~ 7
    // It's easy to check the positive range, the number is the most significant bit + 1.

    int b16, b8, b4, b2, b1, b0;
    x = x ^ (x >> 31); // if x >= 0 then x = x, else x = ~x = -x - 1

    // Use dichotmy to find the most significant bit.
    b16 = !!(x >> 16) << 4; // get 16 if the most significant bit > 16, 0 otherwise.
    x = x >> b16; // if the most significant bit > 16 then left shift 16 bit.
    b8 = !!(x >> 8) << 3;
    x = x >> b8;
    b4 = !!(x >> 4) << 2;
    x = x >> b4;
    b2 = !!(x >> 2) << 1;
    x = x >> b2;
    b1 = !!(x >> 1);
    x = x >> b1;
    b0 = x;

    return b16 + b8 + b4 + b2 + b1 + b0 + 1;
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
    // Split into 3 situations and modify corresponding bits.
    unsigned int mask_exp = 0x7f800000; // 0x0111111110...0
    unsigned int mask_frac = 0x007fffff; // 0x0000000001...1
    unsigned int mask_sign = 0x80000000; // 0x10...0;
    unsigned int expbits = uf & mask_exp;
    unsigned int fracbits = uf & mask_frac;
    unsigned int signbit = uf & mask_sign;
    if (expbits == 0x00000000) {  // denormalized values
        fracbits = fracbits << 1;
        if (fracbits >= 0x400000) {
        expbits = 0x00800000;
        }
        return (signbit | expbits | fracbits);
    } else if (expbits != 0x7f800000) { // normalized values
        if (expbits < 0x7f000000) {
        expbits += (1 << 23);
        } else {
        expbits = 0x7f800000;
        }
    }
    // special values don't need modify
    return (signbit | expbits | fracbits);
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
    // Just implement according to the defination of float.
    unsigned int mask_exp = 0x7f800000; // 0x0111111110...0
    unsigned int mask_frac = 0x007fffff; // 0x0000000001...1
    unsigned int mask_sign = 0x80000000; // 0x10...0;
    unsigned int expbits = uf & mask_exp;
    unsigned int fracbits = uf & mask_frac;
    unsigned int signbit = uf & mask_sign;
    int expbiased = (expbits >> 23) - 127; // get the biased exp number
    int result = 1;
    if (expbits > 0x4f000000) { // if out of range (frac left shift number >= 31)
        return 0x80000000u;
    } else if (expbits < 0x3f800000) { // if float number < 1
        result = 0;
    } else {
        int fracnumber = 1 << 23; // fracnumber = 1 + ?(2^-1) + ?(2^-2) +...
        if (expbiased <= 23) { // check if some fracbits should be discarded
        int i = 0;
        for (; i < expbiased; i++) { // get the remaining bits one by one, discard others.
            int mask_tmp = 1 << (22 - i); // extract one bit
            int checkbit = !((fracbits & mask_tmp) ^ mask_tmp); // get 1 if this bit is 1
            fracnumber += checkbit * mask_tmp;
            }	
            result = fracnumber >> (23 - expbiased);
        } else { // if no need to discard
            fracnumber += fracbits;
            result = fracnumber << (expbiased - 23);
        }
    }

    if (signbit == 0x80000000) { // deal with the signbit.
        result = -result;
    }
    return result;
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
    // Just implement according to the defination of float.
    if (x < -149) { // too small to be represented as a denorm
        return 0;
    } else if (x > 127) { // too large
        return 0x7f800000; // +INF
    }

    if (x <= -126) { // -149 <= x <= -126
        return (1 << (x + 149));
    } else { // -126 < x <= 127
        int esp = x + 127;
        return (esp << 23);
    }
}
