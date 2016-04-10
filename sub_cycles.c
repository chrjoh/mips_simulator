#include "memio.h"
#include "combcirc.h"
#include "sub_cycles.h"



/*
 * Static procedure declarations
 *
 */

static void
set_mem_wb (
    Mem_wb  *mem_wb,		/* register */
    Ex_mem  *ex_mem,		/* register */
    Bus     *bus);		/* signal bus */

static void
set_ex_mem (
    Ex_mem  *ex_mem,		/* register */
    Id_ex   *id_ex,		/* register */
    Bus     *bus);		/* signal bus */

static void
set_id_ex (
    Id_ex           *id_ex,	/* register */
    If_id           *if_id,	/* register */
    Bus             *bus,	/* signal bus */
    Decoded_result  *dec_res,	/* operation instr. */
    Word             regs[]);	/* register */

static void
set_if_id (
    If_id  *if_id,		/* register */
    Bus    *bus,		/* signal bus */
    int    *nop_cnt);		/* number of nop-instr. at program end */



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
    Mem_wb          *mem_wb,	/* register */
    Ex_mem          *ex_mem,	/* register */
    Id_ex           *id_ex,	/* register */
    If_id           *if_id,	/* register */
    Bus             *bus,	/* signal bus */
    Decoded_result  *dec_res,	/* register */
    Word             regs[],	/* register */
    Byte             mem[],	/* primary memory */
    Word            *pc,	/* program counter */
    Word            *pc1,	/* return address for branch and jump */
    int             *nop_cnt)	/* number of nop-instr. at program end */
{

    /*
     * WB, MEM, EX
     *
     * Set the registers MEM_WB, EX_MEM, ID_EX.
     *
     */

    set_mem_wb (mem_wb, ex_mem, bus);
    set_ex_mem (ex_mem, id_ex, bus);
    set_id_ex (id_ex, if_id, bus, dec_res, regs);


    
    /*
     * ID
     *
     * Update register IF_ID.
     * Decide if the return address at jump or branch is going to be
     * stored in register regs.
     *
     */
    
    set_if_id (if_id, bus, nop_cnt);
    
    bus->cm_ctrl = cm_ctrl (if_id->rtype.op, if_id->rtype.funct);
    bus->callmux = callmux (bus->cm_ctrl, if_id->rtype.rd, CM31);
    bus->pcwr = pcwr (if_id->rtype.op, if_id->rtype.funct, if_id->rtype.rt);
    
    if (bus->pcwr == 1)
	regs[bus->callmux] = retaddr (*pc1, RA4);


    
    /*
     * IF
     *
     * Update program counter.
     *
     */
    
    *pc = bus->pcmux;  
}



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
    Mem_wb          *mem_wb,	/* register */
    Ex_mem          *ex_mem,	/* register */
    Id_ex           *id_ex,	/* register */
    If_id           *if_id,	/* register */
    Bus             *bus,	/* signal bus */
    Decoded_result  *dec_res,	/* register */
    Word             regs[],	/* register */
    Word            *pc,	/* program counter */
    Word            *pc1)	/* return address for branch and jump */
{

    /*
     * EX
     *
     * Set the buses from f1mux and f2mux.
     * fwd1, fwd2 does forward checking on the two latest computed results
     * and sets an indicator if one or both of them are going to be
     * written back to register regs.
     *
     */
    
    bus->fwd1 = fwd1 (id_ex->r1addr, ex_mem->memrd, ex_mem->regwrite,
		      ex_mem->waddr, mem_wb->regwrite, mem_wb->waddr);
    
    bus->fwd2 = fwd2 (id_ex->r2addr, id_ex->r2mux, ex_mem->memrd, 
		      ex_mem->regwrite, ex_mem->waddr,
		      mem_wb->regwrite, mem_wb->waddr);
    
    bus->f1mux = f1mux (bus->fwd1, id_ex->r1data, mem_wb->res,
			ex_mem->res_dmar);

    bus->r2mux = r2mux (id_ex->r2mux, id_ex->imm, id_ex->r2data);

    bus->f2mux = f2mux (bus->fwd2, ex_mem->res_dmar, mem_wb->res, bus->r2mux);


    
    /*
     * ID
     *
     * Decode the new instruction.
     * Update the buses i_sext, wamux, smux, ish2.
     *
     */
    
    *dec_res = decode (if_id->rtype.op, if_id->rtype.funct);
    bus->i_sext = i_sext (dec_res->isx, if_id->itype.imm);
    bus->wamux = wamux (dec_res->wamx, if_id->rtype.rd, if_id->rtype.rt);
    
    bus->smux = smux (dec_res->smx, if_id->rtype.shamt,
		      lo5 (regs[if_id->rtype.rs]));
    
    bus->ish2 = ish2 (bus->i_sext);


    
    /*
     * IF
     *
     * Update the buses incr, baddr, jaddr and set pc1.
     * 
     */

    bus->incr = incr (*pc, I4);
    bus->baddr = baddr (bus->ish2, *pc1);
    bus->jaddr = jaddr (maskhi4 (*pc1), tsh2 (if_id->jtype.target));
    *pc1 = bus->incr;
}



/* sub_cycle_three
 * (Falling flank)
 *
 * sub_cycle_three updates register regs (if needed).
 * It also updates the buses dmem and alu.
 *
 */

void
sub_cycle_three (
    Mem_wb  *mem_wb,		/* register */
    Ex_mem  *ex_mem,		/* register */
    Id_ex   *id_ex,		/* register */
    Bus     *bus,		/* signal bus */
    Word     regs[],		/* register */
    Byte     mem[])		/* primary memory */
{

    /*
     * WB
     *
     * If the signal for writing to register is set, then store
     * the result in register MEM_WB in register regs.
     *
     */
    
    if (mem_wb->regwrite)
	regs[mem_wb->waddr] = mem_wb->res;
    


    /*
     * MEM
     *
     * Update dmem bus.
     *
     */
    
    bus->dmem = dmem (mem, ex_mem->res_dmar, ex_mem->smdr, 
		      ex_mem->memwr, ex_mem->memrd, ex_mem->rwsize);
    


    /*
     * EX
     *
     * alu computes its new value and put it on the alu bus.
     *
     */
    
    bus->alu = alu (id_ex->aluop, id_ex->shamt, bus->f2mux, bus->f1mux);
}



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
    Ex_mem  *ex_mem,		/* register */
    Id_ex   *id_ex,		/* register */
    If_id   *if_id,		/* register */
    Bus     *bus,		/* signal bus */
    Word     regs[],		/* register */
    Byte     mem[],		/* primary memory */
    Word    *pc)		/* program counter */
{

    /*
     * MEM
     *
     * If signal for sign-extension is set, then d_sext does
     * a sign-extension on its input parameter.
     *
     */
    
    bus->d_sext = d_sext (bus->dmem, ex_mem->rwsize, ex_mem->d_sext);
    bus->lmux = lmux (lmc_inv(ex_mem->memrd), bus->d_sext, ex_mem->res_dmar);
    


    /*
     * ID
     *
     * In this state are we going to decide if we are going to do a branch,
     * jump or continue the execution in sequential order. The result
     * is put on the pcmux_ctrl bus.
     *
     */
    
    bus->rs_fwd = rs_fwd (if_id->rtype.rs, id_ex->memrd, id_ex->regwrite,
			  id_ex->waddr, ex_mem->waddr, ex_mem->regwrite);
    
    bus->rt_fwd = rt_fwd (if_id->rtype.rt, id_ex->memrd, id_ex->regwrite,
			  id_ex->waddr, ex_mem->waddr, ex_mem->regwrite);
    
    bus->cmux = cmux (bus->rt_fwd, bus->alu, bus->lmux, regs[if_id->rtype.rt]);
    bus->zmux = zmux (bus->rs_fwd, bus->alu, bus->lmux, regs[if_id->rtype.rs]);
    
    bus->cmp  = cmp (bus->zmux, bus->cmux);
    bus->cmpz = cmpz (bus->zmux);
    
    bus->pcmux_ctrl = pcmux_ctrl (if_id->rtype.op, if_id->rtype.funct,
				  if_id->rtype.rt, bus->cmpz,
				  bus->rs_fwd, bus->cmp);



    /*
     * IF
     *
     * Chooses new program counter and put it on pcmux bus.
     * Read next instruction from memory and put it on imem bus.
     *
     */
    
    bus->pcmux = pcmux (bus->pcmux_ctrl, bus->alu, bus->lmux,
			regs[if_id->rtype.rs], bus->baddr,
			bus->jaddr, bus->incr);

    bus->imem = imem (mem, *pc); 
}



/* set_mem_wb
 *
 * set_mem_wb sets the MEM_WB-register.
 *
 */

static void
set_mem_wb (
    Mem_wb  *mem_wb,		/* register */
    Ex_mem  *ex_mem,		/* register */
    Bus     *bus)		/* signal bus */
{
    mem_wb->res      = bus->lmux;
    mem_wb->waddr    = ex_mem->waddr;
    mem_wb->regwrite = ex_mem->regwrite;
}



/* set_ex_mem
 *
 * set_ex_mem sets the EX_MEM-register.
 *
 */

static void
set_ex_mem (
    Ex_mem  *ex_mem,		/* register */
    Id_ex   *id_ex,		/* register */
    Bus     *bus)		/* signal bus */
{
    ex_mem->res_dmar = bus->alu;
    ex_mem->smdr     = id_ex->r2data;
    ex_mem->waddr    = id_ex->waddr;
    ex_mem->regwrite = id_ex->regwrite;
    ex_mem->memwr    = id_ex->memwr;
    ex_mem->memrd    = id_ex->memrd;
    ex_mem->rwsize   = id_ex->rwsize;
    ex_mem->d_sext   = id_ex->d_sext;
}



/* set_id_ex
 *
 * set_id_ex sets the ID_EX-register.
 *
 */

static void
set_id_ex (
    Id_ex           *id_ex,	/* register */
    If_id           *if_id,	/* register */
    Bus             *bus,	/* signal bus */
    Decoded_result  *dec_res,	/* operation instr. */
    Word             regs[])	/* register */
{
    id_ex->r1data   = regs[if_id->rtype.rs];
    id_ex->r2data   = regs[if_id->rtype.rt];
    id_ex->imm      = bus->i_sext;
    id_ex->shamt    = bus->smux;
    id_ex->r1addr   = if_id->rtype.rs;
    id_ex->r2addr   = if_id->rtype.rt;
    id_ex->waddr    = bus->wamux;
    id_ex->regwrite = dec_res->regwr;
    id_ex->aluop    = dec_res->aluop;
    id_ex->r2mux    = dec_res->r2mx;
    id_ex->memwr    = dec_res->memwr;
    id_ex->memrd    = dec_res->memrd;
    id_ex->rwsize   = dec_res->rwsz;
    id_ex->d_sext   = dec_res->dsx;
}



/* set_if_id
 *
 * set_if_id fetches a new instruction from memory
 * and sets the IF_ID-register.
 *
 * If instruction == HALT then put nop-instructions in
 * the instruction register.
 *
 */

static void
set_if_id (
    If_id  *if_id,		/* register */
    Bus    *bus,		/* signal bus */
    int    *nop_cnt)		/* number of nop-instr. at program end */
{
    if_id->ir = bus->imem; 

    if (if_id->ir == HALT  ||  *nop_cnt > 0)
    {
	if_id->ir = NOP;		/* nop-instruction */
	(*nop_cnt)++;
    }

}
