/* This is the headerfile for the memory I/O
 *
 * The memio contains all memory access functions needed
 * to run the mips simulator. It also includes some
 * defines for the reserved memory addresses.  
 *
 * functions:                           Returns:
 * ----------                           --------
 * Word
 * imem (Byte mem[], Word pc)           word from mem[pc]
 *
 * Word
 * dmem (Byte mem[], Word res_dmar,     if memwr or memrd then
 *       Word smdr, Signal memwr,       write or read from mem
 *	 Signal memrd, rwsize);         
 *
 * procedure
 * ---------
 * load_prog (Word mem[], int size,     load file to mem  
 *            int nitems, char *fname)
 *
 */



#ifndef MEMIO_H
#define MEMIO_H

#include "mipstypes.h"



#define   WR_ADDR     16382
#define   RD_ADDR     16380
#define   WR_STATUS   16383
#define   RD_STATUS   16381
#define   RD_ENABLE   0xffffffffu
#define   WR_ENABLE   0xffffffffu



/* imem
 *
 * imem returns a word from primary memory
 *
 */

Word
imem (
    Byte   mem[],
    Word   pc);



/* dmem
 *
 * dmem returns a word, if it is a memory-read then the word returned
 * contains data from the memory. The data in the word returned
 * is either a byte, halfword or fullword. 
 * If it is a memory write then the returned word is the word written
 * to memory.
 * It also checks the address to read from or write to.
 * If the address to read from is RD_ADDR the data is read from stdin
 * If the address to write to is WR_ADDR the data is written to stdout.
 *
 * Returns:   
 *   dmem (a, b, c, WR, NRD, d) == c
 *   dmem (a, b, c, NWR, RD, d) == mem_read (a, b, d)
 *
 * Side-effect:
 *   dmem (a, b, c, WR, NRD, d) == write_mem (a, b, c, d)
 *
 */

Word
dmem (
    Byte     mem[],	  /* primary memory */
    Word     res_dmar,	  /* address to write to or read from  */
    Word     smdr,	  /* data to write to memory or stdout */
    Signal   memwr,	  /* set if memory write */
    Signal   memrd,	  /* set if memory read */
    Signal   rwsize);	  /* if the word read, write is byte, hword or word */



/* load_prog 
 *
 * load_prog read a file to be stored in primary memory
 *
 */    

void
load_prog (
    Byte   mem[],		/* primary memory */
    int    size,		/* size of elements i bytes */
    int    nitems,		/* size of primary memory */
    char  *fname);		/* program name */



#endif
