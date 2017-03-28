#include "ext2.h"

unsigned char *disk;

void remove (struct ext2_disk *disk, const char *file_to_remove){
	inode *parent_directory = traverse_path(file_to_remove, disk->data);
	
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

    remove (disk, file_to_remove);

    assert((1024<<disk->superblock->s_log_block_size)==1024);
    
    return 0;


}



