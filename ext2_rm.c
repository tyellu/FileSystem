#include "ext2.h"

unsigned char *disk;

void remove (struct )

int main(int argc, char **argv){

	//Usage check
	if (argc != 3){
		fprintf(stderr, "Usage: ext2_rm <image file name> <file path>\n ");
		exit(1);
	}

	//open file and get file path
	int fd = open(argv[1], O_RDWR);
	char* filepath = argv[2];

	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
	perror("mmap");
	exit(1);
    }

    struct ext2_group_desc *gdesc = (struct ext2_group_desc *)(disk + EXT2_BLOCK_SIZE*2);
    struct ext2_inode *inode = (struct ext2_inode *)(disk + EXT2_BLOCK_SIZE*5);

}



