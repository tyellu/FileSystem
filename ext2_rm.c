#include "ext2.h"

unsigned char *disk;


void remove_file (unsigned char disk, char *file_to_remove){

	//split the given path
	char file_name[256];

	char *path = file_to_remove;

	inode *parent_directory;

	split(path, file_name);

	if ((parent_directory = traverse_path(path, disk->data))==NULL){
		err(ENOENT, "%s: No such directory\n", path);
	}else if (file_name == "." || file_name == "..")


	dir_entry *entry = file_exists(disk, parent_directory, file_name);
	
	if (entry == NULL){
		err(ENOENT, "%s/%s No such file\n", path, file_name);
	}else if (IS_DIR(retrieve_inode(disk, entry->inode))){
		err(EISDIR, "%s/%s Path is to a directory\n", path, file_name);
	}else{
		
	}
}

int main(int argc, char **argv){
	char *filepath;
	bool a_flag = false;
	int option;

	if (argc < 3) {
        fprintf(stderr,"To run the program ./ext2_rm <image file name> <filepath> \n");
        return 1;
    }

    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }

	//set the superblock

    remove_file (disk, file_to_remove);
    
    return 0;
}



