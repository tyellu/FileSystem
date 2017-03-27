CC=gcc
CFLAGS= -g -Wall

all : ext2_ls ext2_mkdir ext2_rm

ext2_ls : ext2_ls.c ext2_func.o
	$(CC) $(CFLAGS) ext2_func.o ext2_ls.c -o ext2_ls

ext2_mkdir : ext2_mkdir.c ext2_func.o
	$(CC) $(CFLAGS) ext2_func.o ext2_mkdir.c -o ext2_mkdir   

ext2_rm : ext2_rm.c ext2_func.o
	$(CC) $(CFLAGS) ext2_func.o ext2_rm.c -o ext2_rm  

ext2_func.o : ext2_func.c ext2.h
	$(CC) $(CFLAGS) -c ext2_func.c

%.o : %.c ext2.h
	$(CC) $(CFLAGS) -c $<

clean : 
	rm -f *.o ext2_ls ext2_mkdir ext2_rm *~