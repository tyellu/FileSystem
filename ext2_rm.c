#include "ext2.h"

unsigned char *disk;

int main(int argc, char **argv){

	//Usage check
	if (argc != 3){
		fprintf(stderr, "Usage: ext2_rm <image file name> <file path>\n ");
		exit(1);
	}

	//open file and get file path
	int fd = open(argv[1], O_RDWR);
	char* filepath = argv[2];

	if (abscheck(filepath)){
		exit(1);
	}
}

int abscheck (char* path){
	if (path[0] != '/'){
		fprintf(stderr, "Error: file path must be absolute path\n");
		return 1;
	} else {
		return 0;
	}
}