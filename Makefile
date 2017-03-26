CC=gcc
CFLAGS= -g -Wall -Werror
all : ext2_ls

ext2_ls : ext2_ls.c ext2_func.o
	$(CC) $(CFLAGS) ext2_func.o ext2_ls.c -o ext2_ls  

ext2_func.o : ext2_func.c ext2.h
	$(CC) $(CFLAGS) -c ext2_func.c

%.o : %.c ext2.h
	$(CC) $(CFLAGS) -c $<

clean : 
	rm -f *.o ext2_ls *~