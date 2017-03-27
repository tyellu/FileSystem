#include "ext2.h"

int main(int argc, char const *argv[])
{
	
	const char *filepath = argv[2];

	if (argc < 3) {
        fprintf(stderr,"To run the program ./ext2_ls <image file name> <filepath> \n");
        return 1;
    }

    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }

	struct ext2_super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);

	struct ext2_group_desc *gd = ((struct ext2_group_desc *)(disk + (GD_BLOCK_INDEX*EXT2_BLOCK_SIZE)));

	unsigned char *block_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_block_bitmap));

	unsigned char *inode_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_inode_bitmap));

	//get unreserved block index
	int block_index = get_unreserved_bit(block_bm, (sb->s_blocks_count / 8));

	//get unreserved inode index
	int inode_index = get_unreserved_bit(inode_bm, (sb->s_blocks_count / 32));

	printf("filepath: %s\n", filepath);

	printf("block_index: %d\n",block_index);

	printf("inode_index: %d\n",inode_index);


	return 0;
}