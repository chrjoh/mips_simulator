/* This is the header file for the mips simulator.
 *
 * procedure:                           effect:
 * ----------                           -------
 *
 * check_n_argument (int argc, n)       terminates prog if wrong n.o. args
 *
 * run_simulation (Byte mem[])          runs simulation
 *
 */



#ifndef MIPS_H
#define MIPS_H

#include "mipstypes.h"



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
    int   n);    		/* correct number of arguments */



/* run_simulation
 *
 * run_simulation runs the mips processor-cycles
 * It stops three cycles after the HALT.
 *
 */

void
run_simulation (
    Byte    mem[]);




#endif
