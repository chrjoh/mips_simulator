/*
 * NAME:
 *   mips - a program for simulating the mips-processor
 *
 * SYNTAX:
 *   mips <excecutable program>
 *
 * DESCRIPTION:
 *   mips simulates the mips-processor whitout the multiplication
 *   and division removed. Mips read a file given by user.
 *   The file contains program that are going to be run 
 *   on the simulator. 
 *   Mips uses combinatorial circuites to decode and execute 
 *   the instructions from the program running on the simulator.
 *
 * EXAMPLES:
 *   mips aprog.o
 *
 * DIAGNOSTICS:
 *   0           - program executed normaly
 *   1           - wrong number of arguments
 *   2           - error opening file
 *   3           - error reading file
 *   4           - undifined selection in mux
 *
 * NOTES:
 *   We are running three cycles after the HALT-instruction.
 * 
 * VERSION:
 *   1.00
 *
 */



#include <stdio.h>

#include "mipstypes.h"

#include "mips.h"
#include "sub_cycles.h"
#include "memio.h"
#include "mem.h"
#include "combcirc.h"



/*
 * Global declarations
 *
 */

Byte   mem[MEM_SIZE * 4];	/* primary memory */



int
main (
    int   argc,
    char *argv[])
{
    check_n_argument (argc, 2);	   
    load_prog (mem, sizeof(Byte), MEM_SIZE, argv[1]);
    run_simulation (mem);

    return 0;
}



/* check_n_argument
 *
 * check_n_arguments terminates the program if the number
 * of input arguments (including the name of the main prog)
 * from the user do not equals the second parameter.
 *
 */

void
check_n_argument (
    int   argc,			/* number of arguments to main */
    int   n)			/* correct number of arguments */
{
    if (argc != n)
    {
	fprintf (stderr, "Wrong number of arguments.\n");
	exit (1);
    }
}



/* run_simulation
 *
 * run_simulation runs the mips processor-cycles
 * It stops three cycles after the HALT.
 *
 */

void
run_simulation (
    Byte   mem[])
{
    Word             regs[REG_SIZE] = {0}; /* register bank */
    Word             pc = 0;	           /* program counter  */
    Word             pc1 = 0;	           /* next instruction */
    If_id            if_id  = {0};         /* holds the new instruction */
    Id_ex            id_ex  = {0};         /* holds the decoded instr. */
    Ex_mem           ex_mem = {0};         /* holds the result after the alu */
    Mem_wb           mem_wb = {0};         /* holds result to store in regs */
    Bus              bus    = {0};         /* holds all "global" buses */
    Decoded_result   dec_res = {0};        /* holds the new alu-instr. */
    int              nop_cnt = 0;          /* numb. of nop's after halt inst.*/

    while (nop_cnt <= 3)
    {
	sub_cycle_one (&mem_wb, &ex_mem, &id_ex, &if_id, &bus,
		       &dec_res, regs, mem, &pc, &pc1, &nop_cnt);

	sub_cycle_two (&mem_wb, &ex_mem, &id_ex, &if_id, &bus,
		       &dec_res, regs, &pc, &pc1);

	sub_cycle_three (&mem_wb, &ex_mem, &id_ex, &bus, regs, mem);

	sub_cycle_four (&ex_mem, &id_ex, &if_id, &bus, regs, mem, &pc);
    }
}

