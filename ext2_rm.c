#include "ext2.h"

unsigned char *disk;


void remove_file (unsigned char *disk, char *file_to_remove){

	//split the given path
	char file_name[256];

	char *path = file_to_remove;

	inode *parent_directory;

	split(path, file_name);

	if ((parent_directory = traverse_path(path, disk))==NULL){
		err(ENOENT, "%s: No such directory\n", path);
	}else if (!strcmp(file_name, ".") || !strcmp(file_name,"..")){
		err(1, "Cannot delete . or ..\n");
	}


	dir_entry *entry = file_exists(disk, parent_directory, file_name);
	
	if (entry == NULL){
		err(ENOENT, "%s/%s No such file\n", path, file_name);
	}else if (IS_DIR(retrieve_inode(disk, entry->inode))){
		err(EISDIR, "%s/%s Path is to a directory\n", path, file_name);
	}else{
		unsigned int inode = entry->inode;

		entry->inode = 0;
		entry->rec_len = 0;
		entry->name_len =0;
		entry->file_type = 0;
		entry->name[0] = '\0';

		if (--retrieve_inode(disk, entry->inode)->i_links_count == 0){
		    int num_blocks = file_inode->i_blocks / 2;
		    if (num_blocks > 12) {
		      for (i = 0; i < 12; i++) {
		        set_block_bitmap(block_bm_loc, file_inode->i_block[i] - 1, 0);
		      }
		      int master_block_idx = file_inode->i_block[12] - 1;
		      set_block_bitmap(block_bm_loc, master_block_idx, 0);
		      int* blocks = (void*)disk + EXT2_BLOCK_SIZE * (master_block_idx + 1);
		      for (i = 0; i < num_blocks - 13; i++) {
		        set_block_bitmap(block_bm_loc, *blocks - 1, 0);
		        blocks++;
		      }
		    } else {
		      for (i = 0; i < num_blocks; i++) {
		        set_block_bitmap(block_bm_loc, file_inode->i_block[i] - 1, 0);
		      }
		    }
		    set_inode_bitmap(inode_bm_loc, file_inode_idx, 0);
		}
	}
}

int main(int argc, char **argv){

	if (argc < 3) {
        fprintf(stderr,"To run the program ./ext2_rm <image file name> <filepath> \n");
        return 1;
    }
    char *filepath=argv[2];

    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }

	//set the superblock

    remove_file (disk, filepath);
    
    return 0;
}



