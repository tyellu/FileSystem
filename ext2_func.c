#include "ext2.h"


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
  unsigned char byte;
  for (i = 0; i < num_bytes; i++){
    byte = *bm;
    for (j = 0; j < 8; j++) {
      if(((byte >> j) & 1) == 0){
        return(j+(i*8));
      }
    }    
    bm++;
  }

  return -1;
}

void flip_bit(unsigned char * bitmap, unsigned int num_bytes, int index){
  int i, j;
  unsigned char * bm = bitmap;
  for (i = 0; i < num_bytes; i++){
    for (j = 0; j < 8; j++) {
      if((j+(i*8)) == index){
        *bm = *bm ^ (1 << j);
      }
    }    
    bm++;
  }
}


void split(char* file_path, char* file_name) {
  int path_len = strlen(file_path);
  int i = path_len - 1;
  while (file_path[i] != '/') {
    i--;
  }
  int file_name_len = path_len - i - 1;

  strncpy(file_name, file_path + path_len - file_name_len, file_name_len + 1);

  //Set ending char accordingly, special case input '/'
  if (i > 0){
  	file_path[i]='\0';
  }else{
  	file_path[i+1]='\0';
  }
}

dir_entry *file_exists(unsigned char *disk, inode *parent_inode, char *file_name){
	for(int i=0; (i < ((parent_inode->i_blocks / 2)) && (i < 11)); i++){
		dir_entry *curr_dir_entry = (dir_entry *)(disk + 
			(EXT2_BLOCK_SIZE*(parent_inode->i_block[i])));
		unsigned short rec_len = curr_dir_entry->rec_len;
		while((rec_len > 0) && (rec_len < EXT2_BLOCK_SIZE)){
			char name[EXT2_NAME_LEN + 1];
			strncpy(name, curr_dir_entry->name, curr_dir_entry->name_len);
			name[curr_dir_entry->name_len]= '\0';
			if(strcmp(file_name, name) == 0){
				return curr_dir_entry;
			}else{
				curr_dir_entry = (dir_entry *)(disk + 
			(EXT2_BLOCK_SIZE*(parent_inode->i_block[0]))+(rec_len));
				rec_len += curr_dir_entry->rec_len;
			}
		}
	}
	return NULL;
}

inode *retrieve_inode(unsigned char *disk, unsigned int inode_number) {
    inode *curr_inode = (inode *)(disk + (1024*5) + (128*(inode_number-1)));
    return curr_inode;
}

int remove_inode(inode * parent_dir_inode, inode * target_file_inode, char *file_name, unsigned char *disk){

    struct ext2_group_desc * gd = (struct ext2_group_desc *)(disk + GD_BLOCK_INDEX * EXT2_BLOCK_SIZE);
    unsigned int * current_block_pointer = parent_dir_inode->i_block;
    unsigned int current_blk;
    unsigned int inode_bitmap = gd->bg_inode_bitmap;
    unsigned int block_bitmap = gd->bg_block_bitmap;

    
    int i, j, free_blocks = 0;;
    
    //iterate through the direct blocks
    for (i = 0; i < 12 && parent_dir_inode->i_block[i]; i++) {
      	//set current block
        current_blk = parent_dir_inode->i_block[i];
      
        //get directory information
        struct ext2_dir_entry_2 * dir = (struct ext2_dir_entry_2 *)(disk + EXT2_BLOCK_SIZE * current_blk);
        //create previous directory pointer
        struct ext2_dir_entry_2 * previous_directory;

        //starting position 0
        int  position= 0;
        
        //iterate while we are still in the block
        while (position < EXT2_BLOCK_SIZE) {

            //increment position entry
            position += dir->rec_len;

        	//If directory entry is what we are looking for
            if (!strncmp (dir->name, file_name, dir->name_len)) {
            	//decrement link count
                target_file_inode->i_links_count--;

                //If this is last link, we have to change bitmaps
                if (target_file_inode->i_links_count == 0) {

                    //unset bitmap for inodes
                    unset_bitmap((unsigned int *) (disk + EXT2_BLOCK_SIZE * inode_bitmap), 
                    dir->inode - 1);
                    //unset bitmap for blocks
                    unset_bitmap((unsigned int *) (disk + EXT2_BLOCK_SIZE * block_bitmap), 
                        target_file_inode->i_block[i] - 1);

                    //increment free blocks
                    for (j = 0; j < 12 && target_file_inode->i_block[j]; j++) {
                        target_file_inode->i_block[j] = 0;
                        free_blocks++;
                    }

                    //update free blocks and free inode
                    gd->bg_free_blocks_count += free_blocks;
                    gd->bg_free_inodes_count++;

                    //update file inode size and blocks
                    target_file_inode->i_blocks = 0;
                    target_file_inode->i_size = 0;
                }

                //set rec_len to extend previous directory and remove the entry
                previous_directory->rec_len = dir->rec_len + previous_directory->rec_len;
                return 0;
            }

            //if entry is not what we are looking for, increment directory and previous directory
            previous_directory = dir;
            dir = (void *) dir + dir->rec_len;
        }
        current_block_pointer++;
    }
    return 0;
}

void unset_bitmap(unsigned int * bitmap, int index) {
  bitmap += (index / EXT2_INODE_COUNT);
  *bitmap &= (unsigned int)( ~(1 << (index % EXT2_INODE_COUNT)) );
}