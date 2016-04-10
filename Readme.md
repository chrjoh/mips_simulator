## NAME:
    mips - a program for simulating the mips-processor
 
## SYNTAX:
    mips <excecutable program>
 
## DESCRIPTION:
    mips simulates the mips-processor whitout the multiplication
    and division removed. Mips read a file given by user.
    The file contains program that are going to be run 
    on the simulator. 
    Mips uses combinatorial circuites to decode and execute 
    the instructions from the program running on the simulator.
 
 ### EXAMPLES:
    mips aprog.o
 
 ## DIAGNOSTICS:
    0           - program executed normaly
    1           - wrong number of arguments
    2           - error opening file
    3           - error reading file
    4           - undifined selection in mux
 
NOTES:
    We are running three cycles after the HALT-instruction.
  
VERSION:
    1.00
