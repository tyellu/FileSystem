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
<<<<<<< HEAD
void separate(char* path, char* name) {
  int raw_len = strlen(path);
  int i = raw_len - 1;
  while (path[i] != '/') {
    i--;
  }
  int name_len = raw_len - i - 1;
  strncpy(name, path + raw_len - name_len, name_len + 1);
  if (i == 0) { // preserve "/" special case
    path[i + 1] = '\0';
  } else {
    path[i] = '\0';
  }
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
=======
>>>>>>> 7a04dd8fb48a1ae7f7bcf8ba93f72e4e09f3090b
