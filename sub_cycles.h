/* This is the headerfile for handling the sub-cycles.
 * It contains four sub-cycles procedures and a structure for the buses.
 * 
 */

#ifndef SUB_CYCLES_H
#define SUB_CYCLES_H

#include "mipstypes.h"

#include "mem.h"
#include "combcirc.h"


#define   HALT   0xffffffffu
#define   NOP    0


/* struct bus
 *
 * This struct contains signal buses.
 * Here are all the buses that links different states in the pipeline,
 * and buses which is used in different subcycles.
 *
 */

struct Bus
{
    Word     lmux;
    Word     alu;
    Word     i_sext;
    Word     smux;
    Word     wamux;
    Word     f1mux;
    Word     f2mux;
    Word     incr;
    Word     ish2;
    Word     baddr;
    Word     jaddr;
    Word     dmem;
    Word     pcmux_ctrl;
    Word     pcmux;
    Word     imem;
    Signal   cm_ctrl;
    Signal   callmux;
    Signal   pcwr;
    Signal   fwd1;
    Signal   fwd2;
    Signal   r2mux;
    Signal   d_sext;
    Signal   rs_fwd;
    Signal   rt_fwd;
    Signal   cmux;
    Signal   zmux;
    Signal   cmp;
    Signal   cmpz;
};

typedef struct Bus Bus;



/* sub_cycle_one
 * (rising flank)
 *
 * sub_cycle_one updates IF_ID-, ID_EX-, EX_MEM-, MEM_WB-registers
 * and the return address in regs.
 * It also updates the program counter.
 *
 */

void
sub_cycle_one (
    Mem_wb          *mem_wb,
    Ex_mem          *ex_mem,
    Id_ex           *id_ex,
    If_id           *if_id,
    Bus             *bus,
    Decoded_result  *dec_res,
    Word             regs[],
    Byte             mem[],
    Word             *pc,
    Word             *pc1,
    int              *nop_cnt);



/* sub_cycle_two
 * (High clock)
 *
 * sub_cycle_two updates decode and pc1
 * It also updates buses:
 *   f1mux, f2mux, i_sext, wamux, smux, ish2, incr, baddr and jaddr.
 *
 */

void
sub_cycle_two (
    Mem_wb          *mem_wb,
    Ex_mem          *ex_mem,
    Id_ex           *id_ex,
    If_id           *if_id,
    Bus             *bus,
    Decoded_result  *dec_res,
    Word             regs[],
    Word             *pc,
    Word             *pc1);



/* sub_cycle_three
 * (Falling flank)
 *
 * sub_cycle_three updates register regs (if needed).
 * It also updates the buses dmem and alu.
 *
 */

void
sub_cycle_three (
    Mem_wb  *mem_wb,
    Ex_mem  *ex_mem,
    Id_ex   *id_ex,
    Bus     *bus,
    Word     regs[],
    Byte     mem[]);



/* sub_cycle_four
 * (Low clock)
 *
 * sub_cycle_four updates some buses.
 * The buses are:
 *   lmux, pc_ctrl, pcmux and imem.
 *
 */

void
sub_cycle_four (
    Ex_mem  *ex_mem,
    Id_ex   *id_ex,
    If_id   *if_id,
    Bus     *bus,
    Word     regs[],
    Byte     mem[],
    Word    *pc);


#endif
