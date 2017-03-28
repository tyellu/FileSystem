#include "ext2.h"
#define maxchar 10000


bool valid_path(char fp){
	if (fp == '/'){
		return true;
	}
	return false;
}


inode *traverse_path(char *filepath, unsigned char *disk){
	inode *root_inode = (inode *)(disk + (EXT2_BLOCK_SIZE*INODE_TBL_BLOCK) + INODE_STRUCT_SIZE);
	char fp[maxchar];
	strncpy(fp, filepath, strlen(filepath)); 
	if(valid_path(fp[0])){
		inode *curr_inode = root_inode;
		char *str;
		str = strtok (filepath,"/");
		while (str != NULL)
		{
			if(curr_inode->i_mode & EXT2_S_IFDIR){
				dir_entry *curr_dir_entry = (dir_entry *)(disk + 
					(EXT2_BLOCK_SIZE*(curr_inode->i_block[0])));
				unsigned short rec_len = curr_dir_entry->rec_len;
				while((rec_len > 0) && (rec_len <= EXT2_BLOCK_SIZE)){
					char name[EXT2_NAME_LEN + 1];
			    	strncpy(name, curr_dir_entry->name, curr_dir_entry->name_len);
			    	name[curr_dir_entry->name_len]= '\0';
					if(strcmp(str, name) == 0){
						int inode_num = curr_dir_entry->inode;
						curr_inode = (inode *)(disk + 
							(EXT2_BLOCK_SIZE*INODE_TBL_BLOCK) + 
							(INODE_STRUCT_SIZE*(inode_num-1)));
						break;
					}else{
						curr_dir_entry = (dir_entry *)(disk + 
					(EXT2_BLOCK_SIZE*(curr_inode->i_block[0]))+(rec_len));
						rec_len += curr_dir_entry->rec_len;
					}
				}

			}else{
				return NULL;
			}
		    str = strtok (NULL, "/");
		}
		if(strlen(filepath) > 1 && curr_inode == root_inode){
			return NULL;
		}

		return curr_inode;
	}else{
		return NULL;
	}
	
	return NULL;
}

int get_unreserved_bit(unsigned char * bitmap, unsigned int num_bytes){
  int i, j;
  unsigned char * bm = bitmap;
  unsigned char bit;
  for (i = 0; i < num_bytes; i++){
    bit = *bm;
    for (j = 0; j < 8; j++) {
      if(((bit >> j) & 1) == 0){
        // printf("%d\n", (j+(i*8)));
        return(j+(i*8));
      }
    }    
    bm++;
  }

  return -1;
}

super_block *read_superblock(unsigned char *data){
	struct ext2_super_block *superblock = (struct ext2_super_block *) (data + 1024);

	if (superblock->s_magic != 0xef53)
		errx(1, "Superblock signature (%#x) does not match expected ext2 value (%#x)", superblock->s_magic, 0x3f53);

	return superblock;

}

struct ext2_disk *read_disk(const char *name){
	//ensure parameters are correct for the assignment
	assert(sizeof(struct ext2_super_block) == 1024);
	assert(sizeof(struct ext2_block_group)==32);
	assert(sizeof(struct ext2_inode) == 128);

	struct ext2_disk *disk = malloc(sizeof(struct ext2_disk));
	if (disk == NULL)
		err(1, NULL);

	int fd = open(name, O_RDWR);
	if (fd==-1)
		err(1, "%s could not be opened", name);

	//Map the disk into memory using specifications as per the assignment handout
	disk->data = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (disk->data == MAP_FAILED)
		err(1, "%s failed to map into memory", name);

	//set the superblock
	disk->sb = read_superblock(disk->data);

	//get number of block groups (1 for this assignment)
	int block_group_count = (disk->sb->s_blocks_count-1)/disk->sb->s_blocks_per_group;
	assert (block_group_count ==
		((disk->sb->s_inodes_count-1)/disk->sb->s_inodes_per_group));

	struct ext2_block_group **bgs = malloc(block_group_count * sizeof(struct ext2_block_group));
	int i;
	for (i = 0; i < block_group_count; i++){
		bgs[i]=(struct ext2_block_group *) &disk->data[2048+i*32];
	}

	disk->bg = bgs;

}

struct ext2_inode *retrieve_inode(struct ext2_disk *disk, unsigned int block_adr, unsigned int inode_adr) {
    // Find the byte address of the inode table
    int byte_adr = 1024<<disk->sb->s_log_block_size * disk->bg[block_adr]->inode_table;

    return (struct ext2_inode *) &disk->data[byte_adr + (inode_adr - 1) * disk->sb->s_inode_size];
}