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

	//Error check for absolue file path
	if (abscheck(filepath)){
		exit(1);
	}

	//Error check for file type non directory
	if (ftypecheck(filepath)){
		perror(filepath);
		exit(1);
	}
}



int ftypecheck (char* path){
	if (path[strlen(path)-1]=='/'){
		fprintf(stderr, "Error: ");
		errno = EISDIR;
		return 1;
	} else{
		return 0;
	}
}