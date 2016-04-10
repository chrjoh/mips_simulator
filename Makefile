.KEEP_STATE:


CC		= gcc
CFLAGS		= -ansi -pedantic -g -pipe -Wall 

OBJS		= memio.o combcirc.o sub_cycles.o mips.o
SRCS		= $(OBJSFILES:.o=.c)
EXEC		= mips


all:		$(EXEC)

$(EXEC):	$(OBJS)
		$(LINK.c) -o $@  $(OBJS) $(LDLIBS)




,clean:		;rm -f $(OBJS) 
,cleanall:	;rm -f $(OBJS) $(EXEC)
,touch:         ;touch $(SRCS) $(OBJS:.o=.h) $(ADDI)


