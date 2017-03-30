#include "ext2.h"

void remove_file (unsigned char *disk, const char *file_to_remove){

	//split the given path
	char file_name[256];

	//preserve the original file_to_remove
	char *path = malloc(strlen(file_to_remove));
	strcpy(path, file_to_remove);

	inode *parent_directory;

	//split the path and last component
	split(path, file_name);

	//Error check, cant delete '.' or '..'
	if (!strcmp(file_name, ".") || !strcmp(file_name,"..")){
		errx(1, "Cannot delete . or ..");
	//Error check, directory must exist
	} else if ((parent_directory = traverse_path(path, disk))==NULL){
		errx(ENOENT, "%s : No such directory", path);
	}

	//create directory entry for file in parent directory
	dir_entry *entry = file_exists(disk, parent_directory, file_name);

	//If it doesn't exist
	if (entry == NULL){
		errx(ENOENT, "%s : No such file", file_to_remove);
	//If it isn't a regular file
	}else if (!IS_FILE(retrieve_inode(disk, entry->inode))){
		errx(EISDIR, "%s : Path is not to a regualr file", file_to_remove);
	//Otherwise proceed with deleting
	}else{
		remove_inode(parent_directory, retrieve_inode(disk, entry->inode), file_name, disk);
	}
}

int main(int argc, char **argv){

	if (argc < 3) {
        fprintf(stderr,"To run the program ./ext2_rm <image file name> <filepath> \n");
        return 1;
    }

    if (argv[2][0] != '/'){
    	errx(1, "Must be absolute path");
    }

    char *filepath=argv[2];

    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }

    remove_file (disk, filepath);
    
    return 0;
}



