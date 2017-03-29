#include "ext2.h"

int main(int argc, char *argv[])
{
	
	bool s_flag = false;
	char *srcfilepath = argv[2];
	char *diskfilepath = argv[3];

	if (argc < 4) {
        fprintf(stderr,"To run the program ./ext2_ln <image file name> <source filepath> <disk filepath> \n");
        return 1;
    }

    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }

	while((option = getopt(argc, argv, "s:")) != -1){
	    switch (option){
	    case 's':
	    	s_flag = true;
	    	break;
	    default:
	    	break;
	    }
	}


	super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);

	group_desc *gd = ((struct ext2_group_desc *)(disk + (GD_BLOCK_INDEX*EXT2_BLOCK_SIZE)));

	unsigned char *block_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_block_bitmap));

	unsigned char *inode_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_inode_bitmap));

	char src_name[256];
	char *srcpath = srcfilepath;
	split(srcpath, src_name);

	char link_name[256];
	char *linkpath = diskfilepath;
	split(linkpath, link_name);
