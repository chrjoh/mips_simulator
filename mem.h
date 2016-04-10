#ifndef MEM_H
#define MEM_H

#include "mipstypes.h"

#define CM31   31
#define RA4    4
#define I4     4

#define MEM_SIZE   16384
#define REG_SIZE   32



/* 
 * This is a union used for storing new instuctions.
 * There are four different ways to interpret the instruction.
 * 
 */

union If_id
{
    Word   ir;

    struct 
    {
	unsigned   op  : 6;
	unsigned   rs  : 5;
	unsigned   rt  : 5;
	unsigned   imm : 16;
    } itype;

    struct
    {
	unsigned   op     : 6;
	unsigned   target : 26;
    } jtype;

    struct
    {
	unsigned   op    : 6;
	unsigned   rs    : 5;
	unsigned   rt    : 5;
	unsigned   rd    : 5;
	unsigned   shamt : 5;
	unsigned   funct : 6;
    } rtype;
};

typedef   union If_id If_id;



/*
 * The register to hold the decoded instruction.
 *
 */

struct Id_ex
{
    unsigned   r1data   : 32;
    unsigned   r2data   : 32;
    unsigned   imm      : 32;
    unsigned   shamt    : 5;
    unsigned   r1addr   : 5;
    unsigned   r2addr   : 5;
    unsigned   waddr    : 5;
    unsigned   regwrite : 1;
    unsigned   aluop    : 4;
    unsigned   r2mux    : 1;
    unsigned   memwr    : 1;
    unsigned   memrd    : 1;
    unsigned   rwsize   : 2;
    unsigned   d_sext   : 1;
};

typedef   struct Id_ex Id_ex;



/*
 * The register for holding the result from alu and instructions
 * for memory and register access.
 *
 */

struct Ex_mem
{
    unsigned   res_dmar : 32;
    unsigned   smdr     : 32;
    unsigned   waddr    : 5;
    unsigned   regwrite : 1;
    unsigned   memwr    : 1;
    unsigned   memrd    : 1;
    unsigned   rwsize   : 2;
    unsigned   d_sext   : 1;
};

typedef   struct Ex_mem Ex_mem;



/*
 * The register holds the result and information if it is going to be
 * written to the register bank.
 *
 */

struct Mem_wb
{
    unsigned   res      : 32;
    unsigned   waddr    : 5;
    unsigned   regwrite : 1;
};

typedef   struct Mem_wb Mem_wb;


#endif
