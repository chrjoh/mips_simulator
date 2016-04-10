#include <stdio.h>

#include "mipstypes.h"

#include "mem.h"
#include "memio.h"
#include "combcirc.h"



/*
 * Static function and procedure declarations
 *
 */

static void
read_file (
    void  *storage,		/* place to store read data */
    int    size,		/* element size in bytes */
    int    nitems,		/* max number of elements read */
    char  *fname);		/* file name */

static void
write_mem (
    Byte     mem[],		/* primary memory */
    Word     res_dmar,		/* address to write to */
    Word     smdr,		/* data to write */
    Signal   rwsize);		/* size of data */

static Word
mem_read (
    Byte     mem[],		/* primary memory */
    Word     res_dmar,		/* address to read from */
    Signal   rwsize);		/* size of data */

static void
read_file (
    void  *storage,		/* place to store read data */
    int    size,		/* element size in bytes */
    int    nitems,		/* max number of elements read */
    char  *fname);		/* file name */



/* imem
 *
 * imem returns a word from primary memory
 *
 */

Word
imem (
    Byte   mem[],		/* primary memory */
    Word   pc)			/* program counter */
{
    return  *(Word *)(mem + pc);
}



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
    Byte     mem[],		/* primary memory */
    Word     res_dmar,		/* address to write to or read from  */
    Word     smdr,		/* data to write to memory or stdout */
    Signal   memwr,		/* set if memory write */
    Signal   memrd,		/* set if memory read */
    Signal   rwsize)		/* if the data is byte, hword or word */
{
    Word   data = smdr;

    if (memwr == WR  &&  memrd == NRD)
	write_mem (mem, res_dmar, smdr, rwsize);
    
    if (memwr == NWR  &&  memrd == RD)
	data = mem_read (mem, res_dmar, rwsize);

    return data;
}



/* load_prog 
 *
 * load_prog read a file to be stored in primary memory.
 *
 */    

void
load_prog (
    Byte   mem[],		/* primary memory */
    int    size,		/* size of elements i bytes */
    int    nitems,		/* size of primary memory */
    char  *fname)		/* program name */
{
    read_file (mem, size, nitems, fname);
}



/* write_mem
 *
 * write_mem writes data to memory.
 * If the write-address is outside memory then terminate program.
 *
 * if addr >> 2 == RD_ADDR then do nothing
 * else if addr >> 2 == RD_STATUS then do nothing
 * else if addr >> 2 == WR_STATUS then do nothing
 * else if addr >> 2 == WR_ADDR then put data to stdout
 * else if datasize == BSZ then write a byte to memory 
 * else if datasize == HSZ then write a halfword to memory 
 * else if datasize == WSZ then write a word to memory 
 *
 */

static void
write_mem (
    Byte     mem[],		/* primary memory */
    Word     res_dmar,		/* address to write to */
    Word     smdr,		/* data to write */
    Signal   rwsize)		/* size of data */
{
    if (res_dmar >= MEM_SIZE * 4)
    {
	fprintf (stderr, "\nTrying to reach outside memory!\n");
	exit (2);
    }

    switch (res_dmar >> 2)
    {
    case RD_ADDR:		/* write not allowed to this address*/
    case RD_STATUS:		/* write not allowed to this address*/
    case WR_STATUS:		/* write not allowed to this address*/
	break;
	
    case WR_ADDR:		/* check if address reserved for stdout */
	putchar (smdr);
	fflush (stdout);
	break;
      
    default:			/* write data to primary memory */
	if (rwsize == BSZ)
	    *(Byte *)(mem + res_dmar) =  smdr;
	else if (rwsize == HSZ)
	    *(Hword *)(mem + res_dmar) = smdr;
	else
	    *(Word *)(mem + res_dmar) = smdr;
	break;
    }
}



/* mem_read
 *
 * mem_read returns read data from memory.
 * If the read-address is outside memory then terminate program.
 *
 * Returns:
 *   mem_read (a, WR_ADDR << 2, *)   == undefined
 *   mem_read (a, RD_STATUS << 2, *) == RD_ENABLE
 *   mem_read (a, WR_STATUS << 2, *) == WR_ENABLE
 *   mem_read (a, RD_ADDR << 2, *)   == stdin
 *   mem_read (a, b, BSZ)            == *(Byte *)(a + b) 
 *   mem_read (a, b, HSZ)            == *(Hword *)(a + b) 
 *   mem_read (a, b, WSZ)            == *(Word *)(a + b) 
 *
 */

static Word
mem_read (
    Byte     mem[],		/* primary memory */
    Word     res_dmar,		/* address to read from */
    Signal   rwsize)		/* size of data */
{
    Word   data;
  
    if (res_dmar >= MEM_SIZE * 4)
    {
	fprintf (stderr, "\nTrying to reach outside memory!\n");
	exit (2);
    }

    switch (res_dmar >> 2)
    {
    case WR_ADDR:		/* not possible to read from stdout */
	break;
	
    case RD_STATUS:		/* always able to read from stdin */
	data = RD_ENABLE;
	break;

    case WR_STATUS:		/* always able to write to stdout */
	data = WR_ENABLE;
	break;

    case RD_ADDR:		/* address reserved for stdin */
	data = getchar ();
	break;

    default:			/* read data from primary memory */
	if (rwsize == BSZ)
	    data = *(Byte *)(mem + res_dmar);
	else if (rwsize == HSZ)
	    data = *(Hword *)(mem + res_dmar);
	else
	    data = *(Word *)(mem + res_dmar);
	break;
    }

    return data;
}



/* read_file
 *
 * read_file handles file I/O.
 * If an error occurs then the program terminates.
 *
 */

static void
read_file (
    void  *storage,		/* place to store read data */
    int    size,		/* element size in bytes */
    int    nitems,		/* max number of elements read */
    char  *fname)		/* file name */
{
    FILE  *stream;
    int    status;

    stream = fopen (fname, "r");
    
    if (stream == NULL)		           /* check if error opening file */
    {
	fprintf (stderr, "error opening %s\n", fname);
	exit (2);
    }
    
    status = fread (storage, size, nitems, stream);
    
    if (status == 0)		           /* check if error reading file */
    {
	fprintf (stderr, "error reading %s\n", fname);
	exit (3);
    }
    
    fclose (stream);
}
