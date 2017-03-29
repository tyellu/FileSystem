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
		while((rec_len > 0) && (rec_len <= EXT2_BLOCK_SIZE)){
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

