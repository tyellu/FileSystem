#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"

unsigned char *disk;


void print_bitmap(void *bitmap_block, int num_bytes){
	char *array_bitmap = (char *)bitmap_block;

	int r;
	int i;
	int shifter = 0x1;
	for (i = 0;i < num_bytes; i++){
		shifter = 0x1;
		for(r = 0; r < 8; r ++){

			if((array_bitmap[i] & shifter) == shifter){
				printf("1");
			}else{
				printf("0");
			}
			shifter = shifter << 1;
		}
		if(i != num_bytes-1){
			printf(" ");
		}else{
			printf("\n");
		}
	}
}

void print_inodes(struct ext2_inode *inode, struct ext2_super_block *sb) {
	if(inode == NULL){
		return;  // exit immediately, NULL pointer
	}
	int j=0;
	int non_reserve=12;
	int count=1;
	char type = '-';	

	
	for (j; j < 32; j ++) {
		if (((count == 2) || (count >= non_reserve)) && (inode->i_size != 0)) {
				
			if ((inode->i_mode & EXT2_S_IFREG) == EXT2_S_IFREG){
				type = 'f';
			}
			else if ((inode->i_mode & EXT2_S_IFDIR) == EXT2_S_IFDIR) {
				type = 'd';
			}
			else if ((inode->i_mode & EXT2_S_IFLNK) == EXT2_S_IFLNK) {
				type = 's';
			}
				printf("[%d] type: %c size: %d links: %d blocks: %d\n", count, type, inode->i_size, inode->i_links_count, inode->i_blocks);
				printf("[%d] Blocks:  %d\n", count, inode->i_block[0]);
				
		}
		count ++;
		inode = inode + 1;
	}
}

void print_directories(void *directory_zone){
	char type;

	struct ext2_inode *inode = directory_zone - EXT2_BLOCK_SIZE*4;
	int non_reserve = 12;
	int j;
	int count = 1;
	int rec_len_tracker;
	void *disk = directory_zone - EXT2_BLOCK_SIZE*9;
	struct ext2_dir_entry_2 *curr;
	char name[EXT2_NAME_LEN+1];
	int i;
	for (j = 0; j < 32; j ++) {
		if ((((count == 2) || (count >= non_reserve)) && (inode->i_mode != 0)) && (inode->i_mode & EXT2_S_IFREG) != EXT2_S_IFREG){
			printf("   DIR BLOCK NUM: %d (for inode %d)\n", inode->i_block[0], count);
			curr = (struct ext2_dir_entry_2 *)(disk + EXT2_BLOCK_SIZE*inode->i_block[0]);
			rec_len_tracker = ((struct ext2_dir_entry_2 *)(disk + EXT2_BLOCK_SIZE*inode->i_block[0]))->rec_len;
			while(rec_len_tracker <= EXT2_BLOCK_SIZE && rec_len_tracker > 0){
				if ((curr->file_type & EXT2_FT_REG_FILE) == EXT2_FT_REG_FILE){
					type = 'f';
				}
				else if ((curr->file_type & EXT2_FT_DIR) == EXT2_FT_DIR) {
					type = 'd';
				}
				else if ((curr->file_type & EXT2_FT_SYMLINK) == EXT2_FT_SYMLINK) {
					type = 's';
				}
				
				for(i = 0; i < curr->name_len; i++){
					name[i] = curr->name[i];
				}
				name[i] = '\0';
				printf("Inode: %d rec_len: %d name_len: %d type= %c name=%s\n", curr->inode, curr->rec_len, curr->name_len, type, name);
				rec_len_tracker += curr->rec_len;
				curr = (struct ext2_dir_entry_2 *)((char *)curr + curr->rec_len);
			}
			
		}
		count ++;
		inode = inode + 1;
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
	struct ext2_group_desc *gdesc = (struct ext2_group_desc *)(disk + EXT2_BLOCK_SIZE*2);
	struct ext2_inode *inode = (struct ext2_inode *)(disk + EXT2_BLOCK_SIZE*5);
	printf("Inodes: %d\n", sb->s_inodes_count);
	printf("Blocks: %d\n", sb->s_blocks_count);
	printf("Block group:\n");
	printf("    block bitmap: %d\n", gdesc->bg_block_bitmap);
	printf("    inode bitmap: %d\n", gdesc->bg_inode_bitmap);
	printf("    inode table: %d\n", gdesc->bg_inode_table);
	printf("    free blocks: %d\n", gdesc->bg_free_blocks_count);
	printf("    free inodes: %d\n", gdesc->bg_free_inodes_count);
	printf("    used_dirs: %d\n", gdesc->bg_used_dirs_count);
	printf("Block bitmap: ");
	print_bitmap(disk + EXT2_BLOCK_SIZE*3, 16);
	printf("Inode bitmap: ");
	print_bitmap(disk + EXT2_BLOCK_SIZE*4, 4);
	
	printf("\nInodes:\n");
	print_inodes(inode, sb);
	printf("\nDirectory Blocks:\n");
	print_directories(disk + EXT2_BLOCK_SIZE*9);

    return 0;
}