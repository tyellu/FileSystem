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
    printf("\n");      
}


void getempty_bit(unsigned char * bitmap, unsigned int num_bytes){
  int i, j;
  unsigned char * p = bitmap;
  unsigned char buf;
  for (i = 0; i < num_bytes; i++){
    buf = *p;
    int found = 0;
    for (j = 0; j < 8; j++) {
      if(((buf >> j) & 1) == 0){
        printf("%d\n", (j+(i*8)));
        found = 1;
        break;
      }
    }    
    if(found){
      break;
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
    getempty_bit(bm, (sb->s_blocks_count / 8));
    // inode map
    printf("\nInode bitmap:");
    bm = (unsigned char *) (disk + 1024 * gd->bg_inode_bitmap);
    printbm(bm, sb->s_blocks_count / 32);
    getempty_bit(bm, (sb->s_blocks_count / 32));
    struct ext2_inode *inode = (struct ext2_inode *) ((disk+ (1024*5)) + (128));
    char type;
    if (inode->i_mode & EXT2_S_IFREG) {
      type = 'f';
    } else if (inode->i_mode & EXT2_S_IFDIR) {
      type = 'd';
    }
    printf("[%d] type: %c size: %d links: %d blocks: %d\n", 2, type, inode->i_size, inode->i_links_count, inode->i_blocks);
    int block_count = inode->i_blocks / 2;
    int blocks;
    // for (int j = 0; j < block_count; j++) {
    //   if (j > 11) {
    //     break;
    //   }
    //   blocks  = inode->i_block[j];
    // }
    blocks  = inode->i_block[0];
    printf("[%d] Blocks:  %d\n", 2, blocks);
    struct ext2_dir_entry_2 *root = (struct ext2_dir_entry_2 *)(disk + (1024*blocks));
    printf("Inode: %d rec_len : %hu name : %s \n", root->inode, root->rec_len, root->name);
    struct ext2_dir_entry_2 *root2 = (struct ext2_dir_entry_2 *)(disk + (1024*blocks+12));
    printf("Inode: %d rec_len : %hu name : %s \n", root2->inode, root2->rec_len, root2->name);
    return 0;
}
