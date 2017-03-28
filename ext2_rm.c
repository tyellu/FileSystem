#include "ext2.h"

unsigned char *disk;



void remove_file (struct ext2_disk *disk, char *file_to_remove){

	//split the given path
	char file_name[256];
	char *path = file_to_remove;
	inode *parent_directory;
	split(path, file_name);

	if ((parent_directory = traverse_path(path, disk->data))==NULL)
		err(ENOENT, "%s/%s: No such file or directory\n", path, file_name);
}

int main(int argc, char **argv){
	//Usage check
	if (argc != 3){
		fprintf(stderr, "Usage: ext2_rm <image file name> <file path>\n ");
		exit(1);
	}

	char *image_file = argv[1];
    char *file_to_remove = argv[2];
    struct ext2_disk *disk;

    disk=read_disk(image_file);

    remove_file (disk, file_to_remove);

    //ensure block size is 1024 for this assignment
    assert((1024<<disk->sb->s_log_block_size)==1024);
    
    return 0;


}



