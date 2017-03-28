#include "ext2.h"

int main(int argc, char *argv[])
{
	
	char *origfilepath = argv[2];
	char *diskfilepath = argv[3];

	if (argc < 4) {
        fprintf(stderr,"To run the program ./ext2_cp <image file name> <native filepath> <disk filepath> \n");
        return 1;
    }

    int fd = open(argv[1], O_RDWR);
    int file_fd = open(argv[2], O_RDONLY);

    if (file_fd == -1) {
    	perror(argv[2]);
    	exit(ENOENT);
    } 

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }

	struct ext2_super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);

	struct ext2_group_desc *gd = ((struct ext2_group_desc *)(disk + (GD_BLOCK_INDEX*EXT2_BLOCK_SIZE)));

	unsigned char *block_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_block_bitmap));

	unsigned char *inode_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_inode_bitmap));

