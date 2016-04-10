/*
 * This is the headerfile for the combinatorial circuits package
 *
 * The combcirc contains all combinatorial circuits that are
 * needed to run the mips simulator.
 *
 *
 * functions:                           Returns:
 * ----------                           --------
 * Decoded_result                       instructions to alu depending on
 * decode (Signal op, funct)            op and funct
 *
 * Word
 * alu (Signal aluop, shamt,            performes an arithmetic operation
 *      Word r2, r1)                    on r1 and r2
 *
 * Word
 * lmux (Signal s, Word w0, w1)         choses between w0 and w1
 *
 * Word
 * r2mux (Signal s, Word w0, w1)        chooses between w0 and w1
 *
 * Word
 * zmux (Signal s, Word w0, w1, w2)     chooses between w0, w1, w2
 *
 * Word
 * cmux (Signal s, Word w0, w1, w2)     chooses between w0, w1, w2
 *
 * Signal
 * callmux (Signal s, s0, s1)           chooses between s0, s2
 *
 * Word
 * pcmux (Signal s, Word w0, w1,        chooses between w0, w1, w2, w3, w4, w5
 *        Word w2, w3, w4, w5)
 *
 * Signal
 * smux (Signal s, s0, s1)              chooses between s0, s1
 *
 * Signal
 *  wamux (Signal s, s0, s1)            chooses between s0, s1
 *
 * Word
 * f1mux (Signal s, Word w0, w1, w2)    chooses between w0, w1, w2
 *
 * Word
 * f2mux (Signal s, Word w0, w1, w2)    chooses between w0, w1, w2
 *
 * Signal
 * fwd1 (Signal a, nrd, regwr1,         signal ranging from 0 to 2
 *       Signal b, regwr2, c)           depending on input, default = 0
 *
 * Signal
 * fwd2 (Signal a, r2, nrd,             signal ranging from 0 to 2
 *       Signal regwr1, b, regwr2, c)   depending on input, default = 2
 *
 * Word
 * i_sext (Signal s, Hword w)           word sign- or not sign extended
 *
 * Signal
 * cmpz (Word   w)                      compare w to zero
 *
 * Signal
 * cmp (Word w1, w2)                    compare w1 and w2 to each other
 *
 * Signal
 * rs_fwd (Signal a, nrd, regwr1,       signal ranging from 0 to 2
 *         Signal b, c, regwr2)         depending on input, default = 2
 *
 * Signal
 * rt_fwd (Signal a, nrd, regwr1,       signal ranging from 0 to 2
 *         Signal b, c, regwr2)         depending on input, default = 2
 *
 * Signal
 * cm_ctrl (Signal s1, s2)              0 or 1 then comparing s1 and s2 with
 *                                      predefined constants
 *
 * Signal
 * pcwr (Signal op, funct, rt)          0 or 1 then comparing op, funct, rt
 *                                      with predefined constants
 *
 * Signal
 * pcmux_ctrl (Signal op, funct,        signal ranging from 0 to 5
 *             Signal rt,cmpz,          depending on input, default = 5
 *             Signal rs_fwd, cmp)
 *
 * Word
 * ish2 (Word w)                        w shifted left two steps
 *
 * Signal
 * lmc_inv (Signal s)                   s inverted
 *
 * Word
 * tsh2 (Word w)                        w shifted left two steps
 *
 * Word
 * maskhi4 (Word w)                     masks out the four highest bits of w
 *
 * Signal
 * lo5 (Word w)                         masks out the five lowest bits of w
 *
 * Word
 * incr (Word w1, w2)                   w1 + w2
 *
 * Word
 * jaddr (Word w1, w2)                  w1 + w2
 *
 * Word
 * baddr (Word w1, w2)                  w1 + w2
 *
 * Word
 * retaddr (Word w1, w2)                w1 + w2
 *
 * Word
 * d_sext (Word w, Signal size, sext)   w sign- or not signextended
 *                                      depending on if sext is set.
 *
 */



#ifndef COMBCIRC_H
#define COMBCIRC_H

#include "mipstypes.h"



/*
 * Constants for masking
 *
 */

#define    HIGH24 0xffffff00u
#define    HIGH16 0xffff0000u
#define    HIGH4  0xf0000000u
#define    LOW5   0x0000001fu



/*
 * Constants for the output from decode.
 * 
 */



/* wamx */

#define    WA_RD     0
#define    WA_RT     1



/* isx */

/*
 * N.B. The signal is negated to indicate sign extension. 
 * Precisely the opposite of the ex_mem.sext field.
 *
 */

#define    ISX       0  
#define    NISX      1



/* smx */

#define    SHAMT  0
#define    R1LO5  1   /* low order 5 bits of r1data */



/* regwrite */

#define    REGWR   1



/* ALUops */

#define    SLL     0x00	
#define    SRL     0x01	
#define    SRA     0x02	
#define    ADD     0x03	
#define    SUB     0x04	
#define    SLT     0x05	
#define    SLTU    0x06	
#define    AND     0x07	
#define    OR      0x08	
#define    XOR     0x09	
#define    NOR     0x0A	
#define    LUI     0x0B	



/* r2mx */

#define IMM        0
#define R2         1



/* memwr  */

#define    NWR     0
#define    WR      1



/* memrd  */

#define    NRD     0
#define    RD      1



/* rwsz */

#define    BSZ    1
#define    HSZ    2
#define    WSZ    3



/* dsx */

#define    NDSX   0
#define    DSX    1



/*
 * Output from the decode circuit
 * is put into a struct for simplicity.
 * 
 */

struct Decoded_result
{
    unsigned wamx:        1;
    unsigned isx:         1;
    unsigned smx:         1;
    unsigned regwr:       1;
    unsigned aluop:       4;
    unsigned r2mx:        1;
    unsigned memwr:       1;
    unsigned memrd:       1;
    unsigned rwsz:        2;
    unsigned dsx:         1;
};
typedef struct Decoded_result   Decoded_result;



/*
 * Constants for the cmp circuit
 *
 */

#define    NE	0
#define    EQ	1



/*
 * Constants for the cmpz circuit.
 * cmpz only uses the first three,
 * the other masks are for convenient
 * testing of things like ">= 0"
 * by testing (value & GEZ) etc.
 *
 */

#define   EQZ    0x1 	/* equal to zero   */
#define   GTZ    0x2	/* greater than zero */
#define   LTZ    0x4	/* less than zero  */
#define   GEZ    0x3    /* gt or eq to zero (if one of the bits are set)   */
#define   LEZ    0x5	/* lt or eq to zero (if one of the bits are set)   */
#define   NEZ    0x6	/* not equal to zero (if one of the bits are set)  */



/*
 * Constants for the pcmux_ctrl, cm_ctrl and pcwr circuits
 * These are the acual opcodes for the instructions listed
 * listed. Some of them are identical because they come from
 * different subparts of the 32-bit instruction.
 *
 */

#define   J                     2
#define   JAL                   3
#define   BEQ                   4
#define   BNE                   5
#define   BLEZ                  6
#define   BGTZ                  7
#define   JR                    8
#define   JALR                  9
#define   BLTZ                  0
#define   BGEZ                  1
#define   BLTZAL                16
#define   BGEZAL                17

#define   SPECIAL		0
#define   BCOND 		1



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
    Signal   funct);



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
    Word     r1);		/* word to operate on */



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
    Word     w1);



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
    Word     w1);



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
    Word     w2);



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
    Word     w2);



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
    Signal   s1);



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
    Word     w5);



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
    Signal   s1);



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
    Signal   s1);



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
    Word     w2);



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
    Word     w2);



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
    Signal   c);



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
    Signal   c);



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
    Hword    w);



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
    Word   w);



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
    Word   w2);



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
    Signal   regwr2);



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
    Signal   regwr2);



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
    Signal   s2);



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
    Signal   rt);



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
    Signal   cmp);



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
    Word   w);



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
    Signal   s);



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
    Word   w);



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
    Word   w);



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
    Word   w);



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
    Word   w2);



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
    Word   w2);



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
    Word   w2);



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
    Word   w2);



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
    Signal   sext);



#endif
