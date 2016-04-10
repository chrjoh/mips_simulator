#include <assert.h>
#include <stdio.h>

#include "mipstypes.h"

#include "combcirc.h"



/*
 * Static function declarations
 *
 */

static Word
mux (
    Signal   s,			/* selector */
    Word     w0,
    Word     w1,
    Word     w2,
    Word     w3,
    Word     w4,
    Word     w5);

static Signal
fwd (
    Signal   a,
    Signal   nrd,
    Signal   regwr1,
    Signal   b,
    Signal   c,
    Signal   regwr2);

static Word 
sra_aux (
    Word val,
    Signal amt);



typedef Word (*alufunc)(Signal shamt, Word r2, Word r1);



/* 
 * optab holds the different operation codes so that the right
 * operation is chosed in the alu, memread, memwrite and register-
 * write. 
 * 
 * Fields in a struct that we don't care about are
 * marked with a literal zero ('0').
 *
 */

static
Decoded_result
optab[] =
{                                        
    {0},                                                   /*  special       */
    {0},				                   /*  bcond         */
    {0},				                   /*  j             */
    {0},				                   /*  jal           */
    {0},				                   /*  beq           */
    {0},				                   /*  bne           */
    {0},				                   /*  blez          */
    {0},				                   /*  bgtz          */
    {WA_RT, ISX,  0,     REGWR, ADD,  IMM, 0, 0, 0, 0},    /*  addi          */
    {0},				                   /*  ill. instr.   */
    {WA_RT, ISX,  0,     REGWR, SLT,  IMM, 0, 0, 0, 0},    /*  slti          */
    {WA_RT, ISX,  0,     REGWR, SLTU, IMM, 0, 0, 0, 0},    /*  sltiu         */
    {WA_RT, NISX, 0,     REGWR, AND,  IMM, 0, 0, 0, 0},	   /*  andi          */
    {WA_RT, NISX, 0,     REGWR, OR,   IMM, 0, 0, 0, 0},	   /*  ori           */
    {WA_RT, NISX, 0,     REGWR, XOR,  IMM, 0, 0, 0, 0},	   /*  xori          */
    {WA_RT, NISX, 0,     REGWR, LUI,  IMM, 0, 0, 0, 0},    /*  lui           */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {WA_RT, ISX, 0, REGWR, ADD, IMM, NWR, RD, BSZ, DSX},   /*  lb            */
    {WA_RT, ISX, 0, REGWR, ADD, IMM, NWR, RD, HSZ, DSX},   /*  lh            */
    {0},						   /*  ill. instr.   */
    {WA_RT, ISX, 0, REGWR, ADD, IMM, NWR, RD, WSZ, NDSX},  /*  lw            */
    {WA_RT, ISX, 0, REGWR, ADD, IMM, NWR, RD, BSZ, NDSX},  /*  lbu           */
    {WA_RT, ISX, 0, REGWR, ADD, IMM, NWR, RD, HSZ, NDSX},  /*  lhu           */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {WA_RT, ISX, 0,     0, ADD, IMM, WR, NRD, BSZ, NDSX},  /*  sb            */
    {WA_RT, ISX, 0,     0, ADD, IMM, WR, NRD, HSZ, NDSX},  /*  sh            */
    {0},						   /*  ill. instr.   */
    {WA_RT, ISX, 0,     0, ADD, IMM, WR, NRD, WSZ, NDSX},  /*  sw            */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0} 						   /*  ill. instr.   */
};




/* 
 * spectab holds some extra operation codes so that the right
 * operation is chosed in the alu and registerwrite
 * 
 */

static
Decoded_result
spctab[] = 
{
    {WA_RD, 0,   SHAMT, REGWR, SLL,  R2, 0, 0, 0, 0},      /*  sll           */
    {0},						   /*  ill. instr.   */
    {WA_RD, 0,   SHAMT, REGWR, SRL,  R2, 0, 0, 0, 0},      /*  srl           */
    {WA_RD, 0,   SHAMT, REGWR, SRA,  R2, 0, 0, 0, DSX},    /*  sra           */
    {WA_RD, 0,   R1LO5, REGWR, SLL,  R2, 0, 0, 0, 0},      /*  sllv          */
    {0},       						   /*  ill. instr.   */
    {WA_RD, 0,   R1LO5, REGWR, SRL,  R2, 0, 0, 0, 0},      /*  srlv          */
    {WA_RD, 0,   R1LO5, REGWR, SRA,  R2, 0, 0, 0, DSX},    /*  srav          */
    {0},						   /*  jr            */
    {0},						   /*  jalr          */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {0},						   /*  ill. instr.   */
    {WA_RD, 0, 0, REGWR, ADD, R2, 0, 0, 0, 0},		   /*  add           */
    {0},						   /*  ill. instr.   */
    {WA_RD, 0, 0, REGWR, SUB, R2, 0, 0, 0, 0},		   /*  sub           */
    {0},						   /*  ill. instr.   */
    {WA_RD, 0, 0, REGWR, AND, R2, 0, 0, 0, 0},		   /*  and           */
    {WA_RD, 0, 0, REGWR, OR,  R2, 0, 0, 0, 0},		   /*  or            */
    {WA_RD, 0, 0, REGWR, XOR, R2, 0, 0, 0, 0},		   /*  xor           */
    {WA_RD, 0, 0, REGWR, NOR, R2, 0, 0, 0, 0},		   /*  nor           */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0, 0, 0, 0, SLT,  R2, 0, 0, 0, 0},  		   /*  slt           */
    {0, 0, 0, 0, SLTU, R2, 0, 0, 0, 0},		           /*  sltu          */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0},       						   /*  ill. instr.   */
    {0}       						   /*  ill. instr.   */
};



/* sra_aux
 *
 * sra_aux returns a word with the highest bits set if input
 * argument is negative.
 *
 * Call:
 *   sra_aux (a, b)
 *
 * Returns:
 *   if a < 0 or (a >> amt) != 0 then
 *     return a word with the b highest bits set (and the rest to zero)
 *   else
 *     return 0
 *
 */

static
Word 
sra_aux (
    Word val,
    Signal amt)
{
    return ((val >> amt) | (val & WNEG)) ? ~(~0u >> amt) : 0;
}



/*
 * alu functions
 *
 */

Word alu_sll  (Signal shamt, Word r2, Word r1) {return r2 << shamt;}
Word alu_srl  (Signal shamt, Word r2, Word r1) {return r2 >> shamt;}
Word alu_sra  (Signal shamt, Word r2, Word r1) {return sra_aux (r2, shamt);}
Word alu_add  (Signal shamt, Word r2, Word r1) {return r2 + r1;}
Word alu_sub  (Signal shamt, Word r2, Word r1) {return r1 - r2;}
Word alu_slt  (Signal shamt, Word r2, Word r1) {return r1 + WNEG < r2 + WNEG;}
Word alu_sltu (Signal shamt, Word r2, Word r1) {return r1 < r2;}
Word alu_and  (Signal shamt, Word r2, Word r1) {return r1 & r2;}
Word alu_or   (Signal shamt, Word r2, Word r1) {return r1 | r2;}
Word alu_xor  (Signal shamt, Word r2, Word r1) {return r1 ^ r2;}
Word alu_nor  (Signal shamt, Word r2, Word r1) {return ~(r1 | r2);}
Word alu_lui  (Signal shamt, Word r2, Word r1) {return r2 << 16;}



/*
 * alutab is a tabell that holds all alu-functions
 *
 */

static
alufunc
alutab[] =
{
    alu_sll,			/* shift left logical */
    alu_srl,			/* shift right logical */
    alu_sra,			/* shift right arithmetic */
    alu_add,			/* addition */
    alu_sub,			/* subtraction */
    alu_slt,			/* set on less than */
    alu_sltu,			/* set on less than unsigned */
    alu_and,			/* bitwise and */
    alu_or,			/* bitwise or */
    alu_xor,			/* bitwise xor */
    alu_nor,			/* bitwise nor */
    alu_lui			/* loadupper immidiate */
};



/* decode
 *
 * decode returns the translation of the machine code instruction.
 * The result is defined as:
 *
 * Returns:
 *   decode (SPECIAL, funct) == spctab[funct]
 *   decode (op, *) == optab[op]
 *
 */

Decoded_result
decode (
    Signal   op,		
    Signal   funct)
{
    if (op == SPECIAL)
	return spctab[funct];
    else
	return optab[op];
}



/* alu
 * (arithmetic-logic-unit)
 * 
 * alu returns the output from the alu.
 * The result is defined as:
 *
 *  Returns:
 *    alu (0,  shamt, r2, r1) == r2 << shamt
 *    alu (1,  shamt, r2, r1) == r2 >> shamt
 *    alu (2,  shamt, r2, r1) == sra_aux (r2, shamt)  kanske ändras TODO: 
 *    alu (3,  shamt, r2, r1) == r2 + r1
 *    alu (4,  shamt, r2, r1) == r1 - r2
 *    alu (5,  shamt, r2, r1) == r1 + WNEG < r2 + WNEG
 *    alu (6,  shamt, r2, r1) == r1 < r2
 *    alu (7,  shamt, r2, r1) == r1 & r2
 *    alu (8,  shamt, r2, r1) == r1 | r2
 *    alu (9,  shamt, r2, r1) == r1 ^ r2
 *    alu (10, shamt, r2, r1) == ~(r1 | r2)
 *    alu (11, shamt, r2, r1) == r2 << 16
 *
 */

Word
alu (
    Signal   aluop,		/* alu operation */
    Signal   shamt,		/* shift amount */
    Word     r2,		/* word to operate on */
    Word     r1)		/* word to operate on */
{
    return alutab[aluop](shamt, r2, r1);
}



/* mux
 * 
 * mux returns one of six words decided by a signal
 * the result is defined as:
 *
 *  Returns:
 *    mux (0, a, b, c, d, e, f) == a
 *    mux (1, a, b, c, d, e, f) == b
 *    mux (2, a, b, c, d, e, f) == c
 *    mux (3, a, b, c, d, e, f) == d
 *    mux (4, a, b, c, d, e, f) == e
 *    mux (5, a, b, c, d, e, f) == f
 *
 */

static Word
mux (
    Signal   s,			/* selector */
    Word     w0,
    Word     w1,
    Word     w2,
    Word     w3,
    Word     w4,
    Word     w5)
{
    Word   w;			/* storing the choosen word */

    switch (s)			/* select witch word to return */
    {
    case 0:
	w = w0;
	break;
	
    case 1:
	w = w1;
	break;

    case 2:
	w = w2;
	break;

    case 3:
	w = w3;
	break;

    case 4:
	w = w4;
	break;
	
    case 5:
	w = w5;
	break;

    default:
	fprintf (stderr, "Wrong selector in mux");
	exit (4);
    }

    return w;
}



/* lmux
 *
 * lmux returns one of two words decided by a signal
 * the result is defined as:
 *
 *  Returns:
 *    lmux (0, d_sext_bus, ex_mem.res_dmar) ==  d_sext_bus
 *    lmux (1, d_sext_bus, ex_mem.res_dmar) ==  ex_mem.res_dmar
 *
 */


Word
lmux (
    Signal   s,			/* selector */
    Word     w0,
    Word     w1)
{
    return mux (s, w0, w1, 0, 0, 0, 0);	
}



/* r2mux
 *
 * r2mux returns one of two words decided by a signal
 * the result is defined as:
 *
 *  Returns:
 *    r2mux (0, id_ex.imm, id_ex.r2data) ==  id_ex.imm
 *    r2mux (1, id_ex.imm, id_ex.r2data) ==  id_ex.r2data
 *
 */
  
Word
r2mux (
    Signal   s,			/* selector */
    Word     w0,
    Word     w1)
{
    return mux (s, w0, w1, 0, 0, 0, 0);
}



/* zmux
 *
 * zmux returns one of three words decided by a signal
 * the result is defined as:
 *
 *  Returns:
 *    zmux (0, a, b, c)  == a
 *    zmux (1, a, b, c)  == b
 *    zmux (2, a, b, c)  == c
 *
 */

Word
zmux (
    Signal   s,			/* selector */
    Word     w0,
    Word     w1,
    Word     w2)
{
    return mux (s, w0, w1, w2, 0, 0, 0);
}



/* cmux
 *
 * cmux returns one of three words decided by a signal
 * the result is defined as:
 *
 *  Returns:
 *    cmux (0, a, b, c)  == a
 *    cmux (1, a, b, c)  == b
 *    cmux (2, a, b, c)  == c
 *
 */

Word
cmux (
    Signal   s,			/* selector */
    Word     w0,
    Word     w1,
    Word     w2)
{
    return mux (s, w0, w1, w2, 0, 0, 0);
}



/* callmux
 *
 * callmux returns one of two signals decided by a signal
 * the result is defined as:
 *
 *  Returns:
 *    callmux (0, if_id.rd, cm31) == if_id.rd
 *    callmux (1, if_id.rd, cm31) == cm31
 *
 */

Signal
callmux (
    Signal   s,			/* selector */
    Signal   s0,
    Signal   s1)
{
    return mux (s, s0, s1, 0, 0, 0, 0);
}



/* pcmux
 *
 * pcmux returns one of six words decided by a signal
 * the result is defined as:
 *
 *  Returns:
 *    pcmux (0, a, b, c, d, e, f) == a
 *    pcmux (1, a, b, c, d, e, f) == b
 *    pcmux (2, a, b, c, d, e, f) == c
 *    pcmux (3, a, b, c, d, e, f) == d
 *    pcmux (4, a, b, c, d, e, f) == e
 *    pcmux (5, a, b, c, d, e, f) == f
 *
 */

Word
pcmux (
    Signal   s,			/* selector */
    Word     w0,
    Word     w1,
    Word     w2,
    Word     w3,
    Word     w4,
    Word     w5)
{
    return mux (s, w0, w1, w2, w3, w4, w5);
}



/* smux
 *
 * smux returns one of two signals decided by a signal
 * the result is defined as:
 *
 *  Returns:
 *    smux (0, if_id.shamt, lo5_bus) == if_id.shamt
 *    smux (1, if_id.shamt, lo5_bus) == lo5_bus
 *
 */

Signal
smux (
    Signal   s,			/* selector */
    Signal   s0,
    Signal   s1)
{
    return mux (s, s0, s1, 0, 0, 0, 0);
}



/* wamux
 *
 * wamux returns one of two signals decided by a signal
 * the result is defined as:
 *
 *  Returns:
 *    wamux (0, if_id.rd, if_id.rt) == if_id.rd
 *    wamux (1, if_id.rd, if_id.rt) == if_id.rt
 *
 */

Signal
wamux (
    Signal   s,			/* selector */
    Signal   s0,
    Signal   s1)
{
    return mux (s, s0, s1, 0, 0, 0, 0);
}



/* f1mux
 *
 * f1mux returns one of three words decided by a signal
 * the result is defined as:
 *
 *  Returns:
 *    f1mux (0, id_ex.r1data, mem_wb.res, ex_mem.res_dmar) == id_ex.r1data
 *    f1mux (1, id_ex.r1data, mem_wb.res, ex_mem.res_dmar) == mem_wb.res
 *    f1mux (2, id_ex.r1data, mem_wb.res, ex_mem.res_dmar) == ex_mem.res_dmar
 *
 */

Word
f1mux (
    Signal   s,			/* selector */
    Word     w0,
    Word     w1,
    Word     w2)
{
    return mux (s, w0, w1, w2, 0, 0, 0);
}



/* f2mux
 *
 * f2mux returns one of three words decided by a signal
 * the result is defined as:
 *
 *  Returns:
 *    f2mux (0, ex_mem.res_dmar, mem_wb.res, r2mux_bus) == ex_mem.res_dmar
 *    f2mux (1, ex_mem.res_dmar, mem_wb.res, r2mux_bus) == mem_wb.res
 *    f2mux (2, ex_mem.res_dmar, mem_wb.res, r2mux_bus) == r2mux_bus
 *
 */

Word
f2mux (
    Signal   s,			/* selector */
    Word     w0,
    Word     w1,
    Word     w2)
{
    return mux (s, w0, w1, w2, 0, 0, 0);
}



/* fwd1
 *
 * fwd1 returns a signal decided by how the input parameters
 * are set to each other.
 * The result is defined as:
 *
 *  Returns:
 *    fwd1 (0, *, *, *, *, *) 	        == 0 	
 *
 *    a != b  &&  a != c  &&  a != 0:
 *
 *    fwd1 (a,   *,     *, b,     *, c) == 0 	
 *    fwd1 (a,   *,     *, b, REGWR, a) == 1 	
 *    fwd1 (a, NRD, REGWR, a,     *, *) == 2 	
 *   
 *    else fwd1 (*, *, *, *, *, *)      == 0
 *
 */

Signal
fwd1 (
    Signal   a,
    Signal   nrd,
    Signal   regwr1,
    Signal   b,
    Signal   regwr2,
    Signal   c)
{
    Signal   s = 0;			/* store the signal to return */

    if (a == 0)
	s = 0;
    
    if (a != 0  &&  a != b  &&  a != c)
	s = 0;

    if (a != 0  &&  a != b  &&  regwr2 == REGWR  &&  a == c)
	s = 1;

    if (a != 0  &&  nrd == NRD  &&  regwr1 == REGWR  &&  a == b)
	s = 2;

    return s;
}



/* fwd2
 *
 * fwd1 returns a signal decided by how the input parameters
 * are set to each other.
 * The result is defined as:
 *
 *  Returns:
 *    fwd2 (0,   *, *, *, *, *, *) 	    == 2
 *    fwd2 (*, IMM, *, *, *, *, *)   	    == 2
 *
 *    a != b  &&  a != c  &&  a != 0:
 *
 *    fwd2 (a, R2, NRD, REGWR, a,     *, *) == 0
 *    fwd2 (a, R2,   *,     *, b, REGWR, a) == 1
 *    fwd2 (a,  *,   *,     *, b,     *, c) == 2
 *
 *    else fwd2 (*, *, *, *, *, *)          == 2
 *
 */

Signal
fwd2 (
    Signal   a,
    Signal   r2,
    Signal   nrd,
    Signal   regwr1,
    Signal   b,
    Signal   regwr2,
    Signal   c)
{
    Signal   s = 2;			/* store the signal to return */

    if (a == 0  ||  r2 == IMM)
	s = 2;

    if (a != 0  &&  r2 == R2  &&  nrd == NRD
	&&  regwr1 == REGWR  &&  a == b)
    {
	s = 0;
    }

    if (a != 0  &&  r2 == R2  &&  a != b
	&&  regwr2 == REGWR  &&  a == c)
    {
	s = 1;
    }

    if (a != 0  &&  a != b  &&  a != c)
	s = 2;

    return s;
}



/* i_sext
 *
 * i_sext returns a word sign extended or not sign extended
 * decided by a signal.
 * The result is defined as:
 *
 *  Returns:
 *    i_sext (ISX, if_id.imm) == sign-extended result
 *    i_sext (NISX, if_id.imm) == non-sign-extended result
 *
 */

Word
i_sext (
    Signal   s,
    Hword    w)	
{
    Word res = w;

    if ((s == ISX)  &&  (w & HNEG) != 0)
	res = w | HIGH16;

    return res;
}



/* cmpz
 *
 * cmpz returns a signal depending on the input parameter
 * was greater, equal or less then zero.
 * The result is defined as:
 * 
 *  Returns:
 *    a == 0 :	cmpz (a) == EQZ
 *    a >  0 :	cmpz (a) == GTZ
 *    a <  0 :	cmpz (a) == LTZ
 *
 */

Signal
cmpz (
    Word   w)
{
    Signal   s;			
   
    if (w == 0)
	s = EQZ;

    if ((w & WNEG) == 0  &&  (w > 0))
	s = GTZ;

    if ((w & WNEG) != 0)
	s = LTZ;

     return s;
}



/* cmp
 *
 * cmp returns a signal depending on if the input
 * parameters are equal or not equal.
 * The result is defined as:
 *
 *  Returns:
 *    a != b 
 *
 *	 cmp (a, a)	== EQ
 *	 cmp (a, b)	== NE
 *
 */

Signal
cmp (
    Word   w1,
    Word   w2)
{
    return (w1 == w2) ? EQ : NE;
}



/* fwd
 *
 * fwd returns a signal decided by how the input parameters
 * are set to each other.
 * The result is defined as:
 *
 *  Returns:
 *    fwd (0, *, *, *, *, *)	 	 ==  2  
 *
 *    a != b  &&  a != c  &&  a != 0
 *
 *    fwd (a, NRD, REGWR, a, *, *)	 ==  0
 *    fwd (a, RD,  *,     *, a, REGWR)	 ==  1
 *    fwd (a, *,   *,     b, a, REGWR)	 ==  1
 *    fwd (a, RD,  *,     *, b, *)	 ==  2
 *    fwd (a,  *,  *,     b, c, *)	 ==  2
 *
 *    fwd (*, *, *, *, *, *)             == -1 else
 *
 */

static Signal
fwd (
    Signal   a,
    Signal   nrd,
    Signal   regwr1,
    Signal   b,
    Signal   c,
    Signal   regwr2)
{
    Signal   s = -1;			/* store the signal to return */
    
    if (a == 0)
	s = 2;

    if (a != 0  &&  nrd == NRD  &&  regwr1 == REGWR  &&  a == b)
	s = 0;

    if (a != 0  &&  nrd == RD  &&  a == c  &&  regwr2 == REGWR)
	s = 1;

    if (a != 0  &&  a != b  &&  a == c  &&  regwr2 == REGWR)
	s = 1;

    if (a != 0  &&  nrd == RD  &&  a != c)
	s = 2;
    
    if (a != b  &&  a != c  && a != 0)
	s = 2;
    
    return s;
}



/* rs_fwd
 *
 * rs_fwd returns a signal decided by how the input parameters
 * are set to each other
 * the result is defined as:
 *
 *  Returns:
 *    rs_fwd (0, *, *, *, *, *)	 	         ==  2   
 *
 *    a != b  &&  a != c  &&  a != 0
 *
 *    rs_fwd (a, NRD, REGWR, a, *, *)		 ==  0
 *    rs_fwd (a, RD,  *,     *, a, REGWR)	 ==  1
 *    rs_fwd (a, *,   *,     b, a, REGWR)	 ==  1
 *    rs_fwd (a, RD,  *,     *, b, *)		 ==  2
 *    rs_fwd (a,  *,  *,     b, c, *)		 ==  2
 *
 *    rt_fwd (*, *, *, *, *, *)                  ==  2 else
 *
 */

Signal
rs_fwd (
    Signal   a,
    Signal   nrd,
    Signal   regwr1,
    Signal   b,
    Signal   c,
    Signal   regwr2)
{
    Signal   s;

    s = fwd (a, nrd, regwr1, b, c, regwr2);
   
    return (s == -1) ? 2 : s; 
}



/* rt_fwd
 *
 * rt_fwd returns a signal decided by how the input parameters
 * are set to each other
 * the result is defined as:
 *
 *  Returns:
 *    rt_fwd (0, *, *, *, *, *)	 	         ==  2   
 *
 *    a != b  &&  a != c  &&  a != 0
 *
 *    rt_fwd (a, NRD, REGWR, a, *, *)		 ==  0
 *    rt_fwd (a, RD,  *,     *, a, REGWR)	 ==  1
 *    rt_fwd (a, *,   *,     b, a, REGWR)	 ==  1
 *    rt_fwd (a, RD,  *,     *, b, *)		 ==  2
 *    rt_fwd (a,  *,  *,     b, c, *)		 ==  2
 *
 *    rt_fwd (*, *, *, *, *, *)                  ==  2 else
 *
 */

Signal
rt_fwd (
    Signal   a,
    Signal   nrd,
    Signal   regwr1,
    Signal   b,
    Signal   c,
    Signal   regwr2)
{
    Signal   s;

    s = fwd (a, nrd, regwr1, b, c, regwr2);

    return (s == -1) ? 2 : s; 
}



/* cm_ctrl
 *
 * cm_ctrl returns a signal
 * the result is defined as:
 *
 *  Returns:
 *    cm_ctrl (SPECIAL, JALR)	  == 0
 *    cm_ctrl (a, b)  		  == 1  else
 *
 */

Signal
cm_ctrl (
    Signal   s1,
    Signal   s2)
{
    return ((s1 == SPECIAL)  &&  (s2 == JALR)) ? 0 : 1;
}



/* pcwr
 *
 * pcwr returns a signal
 * the result is defined as:
 *
 *  Returns:
 *    pcwr (JAL,        *,      *)		== 1		
 *    pcwr (SPECIAL, JALR,      *)  		== 1		
 *    pcwr (BCOND,      *, BLTZAL)		== 1		
 *    pcwr (BCOND,      *, BGEZAL)		== 1		
 *    pcwr (if_id.op, if_id.funct, if_id.rt)	== 0 else
 *
 */

Signal
pcwr (
    Signal   op,
    Signal   funct,
    Signal   rt)
{
    Signal   s = 0;		/* store the signal to return */

    if (op == JAL)
	s = 1;

    if (op == SPECIAL  &&  funct == JALR)
	s = 1;

    if (op == BCOND  &&  rt == BLTZAL)
	s = 1;

    if (op == BCOND  &&  rt == BGEZAL)
	s = 1;

    return s;
}



/* pcmux_ctrl
 *
 * pcmux_ctrl returns a signal
 * the result is defined as:
 *
 *  Returns:
 *    pcmux_ctrl (SPECIAL, JR,    *,      *,   0, *)	 == 0 		
 *    pcmux_ctrl (SPECIAL, JALR,  *,      *,   0, *)	 == 0 		
 *
 *    pcmux_ctrl (SPECIAL, JR,    *,      *,   1, *)	 == 1 		
 *    pcmux_ctrl (SPECIAL, JALR,  *,      *,   1, *)	 == 1  		
 *
 *    pcmux_ctrl (SPECIAL, JR,    *,      *,   2, *)	 == 2 		
 *    pcmux_ctrl (SPECIAL, JALR,  *,      *,   2, *)	 == 2		
 *
 *    pcmux_ctrl (BCOND,   *,     BLTZ,   LTZ, *, *)	 == 3		
 *
 *    pcmux_ctrl (BCOND,   *,     BGEZ,   EQZ, *, *)	 == 3		
 *    pcmux_ctrl (BCOND,   *,     BGEZ,   GTZ, *, *)	 == 3		
 *
 *    pcmux_ctrl (BCOND,   *,     BLTZAL, LTZ, *, *)     == 3		
 *
 *    pcmux_ctrl (BCOND,   *,     BGEZAL, EQZ, *, *)     == 3		
 *    pcmux_ctrl (BCOND,   *,     BGEZAL, GTZ, *, *)     == 3		
 *
 *    pcmux_ctrl (BEQ,     *,     *,      *,   *, EQ)	 == 3		
 * 
 *    pcmux_ctrl (BNE,     *,     *,      *,   *, NE)	 == 3		
 *
 *    pcmux_ctrl (BLEZ,    *,     *,      EQZ, *, *)	 == 3		
 *    pcmux_ctrl (BLEZ,    *,     *,      LTZ, *, *)	 == 3
 *
 *    pcmux_ctrl (BGTZ,    *,     *,      GTZ, *, *)	 == 3		
 *
 *    pcmux_ctrl (J,       *,     *,      *,   *, *)	 == 4		
 *    pcmux_ctrl (JAL,     *,     *,      *,   *, *)	 == 4		
 *
 *    pcmux_ctrl (*,       *,     *,      *,   *, *)	 == 5 else
 *
 */

Signal
pcmux_ctrl (
    Signal   op,
    Signal   funct,
    Signal   rt,
    Signal   cmpz,
    Signal   rs_fwd,
    Signal   cmp)
{
    Signal   s = 5;		/* store the signal to return */

    if (op == SPECIAL  &&  funct == JR  &&  rs_fwd == 0)
	s = 0;

    if (op == SPECIAL  &&  funct == JALR  &&  rs_fwd == 0)
	s = 0;

    if (op == SPECIAL  &&  funct == JR  &&  rs_fwd == 1)
	s = 1;

    if (op == SPECIAL  &&  funct == JALR  &&  rs_fwd == 1)
	s = 1;

    if (op == SPECIAL  &&  funct == JR  &&  rs_fwd == 2)
	s = 2;

    if (op == SPECIAL  &&  funct == JALR  &&  rs_fwd == 2)
	s = 2;

    if (op == BCOND  &&  rt == BLTZ  &&  cmpz == LTZ)
	s = 3;

    if (op == BCOND  &&  rt == BGEZ  &&  cmpz == EQZ)
	s = 3;

    if (op == BCOND  &&  rt == BGEZ  &&  cmpz == GTZ)
	s = 3;

    if (op == BCOND  &&  rt == BLTZAL  &&  cmpz == LTZ)
	s = 3;

    if (op == BCOND  &&  rt == BGEZAL  &&  cmpz == EQZ)
	s = 3;

    if (op == BCOND  &&  rt == BGEZAL  &&  cmpz == GTZ)
	s = 3;

    if (op == BEQ  &&  cmp == EQ)
	s = 3;

    if (op == BNE  &&  cmp == NE)
	s = 3;

    if (op == BLEZ  &&  cmpz == EQZ)
	s = 3;

    if (op == BLEZ  &&  cmpz == LTZ)
	s = 3;

    if (op == BGTZ  &&  cmpz == GTZ)
	s = 3;

    if (op == JAL  ||  op == J)
	s = 4;

    return s;
}



/* ish2
 *
 * ish2 returns a word shifted left two steps
 * the result is defined as:
 *
 *  Returns:
 *    ish2 (w) == w << 2
 *
 */

Word
ish2 (
    Word   w)
{
    return (w << 2);
}



/* lmc_inv
 *
 * lmc_inv returns a inverted signal.
 * the result is defined as:
 *
 *   Returns:
 *     imc_inv (1) == 0
 *     imc_inv (*) == 1
 *
 */

Signal
lmc_inv (
    Signal   s)
{
    return (s == 1) ? 0 : 1;
}



/* tsh2
 *
 * tsh2 returns a word shifted left two steps
 * the result is defined as:
 *
 *  Returns:
 *    tsh2 (w) == w << 2
 *
 */

Word
tsh2 (
    Word   w)
{
    return (w << 2);
}




/* maskhi4
 *
 * maskhi4 returns a word with the four highest bits 
 * of the input word, the four bits will be stored in 
 * the four highest bits of the new word
 *
 *
 */

Word
maskhi4 (
    Word   w)
{
    return w & HIGH4;	/* mask out the 4 highest bits */
}



/* lo5
 *
 * lo5 returns a word with the five lowest bits 
 * of the input word, the five bits will be stored in 
 * the five lowest bits of the new word
 *
 *
 *
 */

Signal
lo5 (
    Word   w)
{
    return  w & LOW5;		/* mask out the 5 lowest bits */
}



/* incr
 *
 * incr returns a word
 * the result is defined as:
 *
 *  Returns:
 *    incr (a, b) == a + b
 *
 */

Word
incr (
    Word   w1,
    Word   w2)
{
    return w1 + w2;
}



/* jaddr
 *
 * jaddrr returns a word
 * the result is defined as:
 *
 *  Returns:
 *    jaddrr (a, b) == a + b
 *
 */

Word
jaddr (
    Word   w1,
    Word   w2)
{
    return w1 + w2;
}



/* baddr
 *
 * baddrr returns a word
 * the result is defined as:
 *
 *  Returns:
 *    baddrr (a, b) == a + b
 *
 */

Word
baddr (
    Word   w1,
    Word   w2)
{
    return w1 + w2;
}


/* retaddr
 *
 * retaddr returns a word
 * the result is defined as:
 *
 *  Returns:
 *    retaddr (a, b) == a + b
 *
 */

Word
retaddr (
    Word   w1,
    Word   w2)
{
    return w1 + w2;
}



/* d_sext
 *
 * d_sext returns a word either sign extended or not
 * sign extended decided by a signal
 * the result is defined as:
 *
 *  Call:
 *    d_sext (w, size, sext)		
 *
 *  Returns:
 *    if (sext == DSX) && (size == BSZ) 
 *     then return (w sign extended)
 *
 *    if (sext == DSX) && (size == HSZ) 
 *     then return (w sign extended)
 *   
 *    else
 *        return (w unchanged)
 *
 */

Word
d_sext (
    Word     w,
    Signal   size,
    Signal   sext)
{
    if (sext == DSX  &&  size == BSZ)
	if ((w & BNEG) != 0)
	    w |= HIGH24;	/* Signextended byte */

    if (sext == DSX  &&  size == HSZ)
	if ((w & HNEG) != 0)
	    w |= HIGH16;	/* Signextended halfword */
    
    return w;
}
 
