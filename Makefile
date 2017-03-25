all : ext2_ls

ext2_ls : ext2_ls.c ext2_func.o
	gcc -Wall -g ext2_func.o ext2_ls.c -o ext2_ls  

ext2_func.o : ext2_func.c ext2.h
	gcc -g -Wall -c ext2_func.c

%.o : %.c ext2.h
	gcc -Wall -g -c $<

clean : 
	rm -f *.o ext2_ls *~