#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"

unsigned char *disk;


void printgd(unsigned char *disk){
   struct ext2_group_desc *gd;
   gd = ((struct ext2_group_desc *)(disk + (2*(1024))));
   printf("     block bitmap: %d\n",gd->bg_block_bitmap);
   printf("     inode bitmap: %d\n",gd->bg_inode_bitmap);
   printf("     inode table: %d\n",gd->bg_inode_table);
   printf("     free blocks: %d\n",gd->bg_free_blocks_count);
   printf("     free inodes: %d\n",gd->bg_free_inodes_count);
   printf("     used_dirs: %d\n",gd->bg_used_dirs_count);

}

void printbm(unsigned char * bitmap, unsigned int num_bytes) {
    int i, j;
    unsigned char * p = bitmap;
    unsigned char buf;
    for (i = 0; i < num_bytes; i++) {
      printf(" ");
      buf = *p;
      for (j = 0; j < 8; j++) {
        printf("%d", (buf >> j) & 1);
      }
      p++;
    }      
}



int main(int argc, char **argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: readimg <image file name>\n");
        exit(1);
    }
    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
	perror("mmap");
	exit(1);
    }

    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
    printf("Inodes: %d\n", sb->s_inodes_count);
    printf("Blocks: %d\n", sb->s_blocks_count);
    printf("Block group:\n");
    struct ext2_group_desc *gd;
    gd = ((struct ext2_group_desc *)(disk + (2*(1024))));
    printgd(disk);
    printf("Block bitmap:");
    unsigned char * bm = (unsigned char *)(disk + 1024 * gd->bg_block_bitmap);
    printbm(bm, (sb->s_blocks_count / 8));

    
    return 0;
}
