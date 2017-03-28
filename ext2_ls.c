#include "ext2.h"

unsigned char *disk;

int main(int argc, char *argv[])
{

	char *filepath;
	bool a_flag = false;
	int option;

	if (argc < 3) {
        fprintf(stderr,"To run the program ./ext2_ls <image file name> <filepath> \n");
        return 1;
    }

    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }
    
	//Gets the options chosen from command line arguments
	while((option = getopt(argc, argv, "a:")) != -1){
	    switch (option){
	    case 'a':
	    	a_flag = true;
	    	filepath = malloc((strlen(optarg) + 1) * sizeof(char *));
      		strcpy(filepath, optarg);
	    	break;
	    default:
	    	break;
	    }
	}

	if(!a_flag){
		filepath =  argv[2];
	}

	char fp[10000];
	strncpy(fp, filepath, strlen(filepath));
	inode *dir_inode = traverse_path(filepath, disk);

	if(dir_inode != NULL && (dir_inode->i_mode & EXT2_S_IFDIR)){
		dir_entry *curr_dir_entry;
		unsigned short rec_len;
		if(!a_flag){
			curr_dir_entry = (dir_entry *)(disk + 
				(EXT2_BLOCK_SIZE*(dir_inode->i_block[0])));
			rec_len = 0;
			while((rec_len >= 0) && (rec_len < EXT2_BLOCK_SIZE)){
				char name[EXT2_NAME_LEN + 1];
			    strncpy(name, curr_dir_entry->name, curr_dir_entry->name_len);
			    name[curr_dir_entry->name_len]= '\0';
			    if((strncmp(name,".", strlen(".")) == 0) || (strncmp(name,"..", strlen("..")) == 0)){
			    	rec_len += curr_dir_entry->rec_len;
					curr_dir_entry = (dir_entry *)(disk + 
						(EXT2_BLOCK_SIZE*(dir_inode->i_block[0]))+(rec_len));
			    }else{
			    	printf("%s\n",name);
					rec_len += curr_dir_entry->rec_len;
					curr_dir_entry = (dir_entry *)(disk + 
						(EXT2_BLOCK_SIZE*(dir_inode->i_block[0]))+(rec_len));
			    }

			}
			
		}else{
			curr_dir_entry = (dir_entry *)(disk + 
				(EXT2_BLOCK_SIZE*(dir_inode->i_block[0])));
			rec_len = 0;
			while((rec_len >= 0) && (rec_len < EXT2_BLOCK_SIZE)){
				char name[EXT2_NAME_LEN + 1];
			    strncpy(name, curr_dir_entry->name, curr_dir_entry->name_len);
			    name[curr_dir_entry->name_len]= '\0';
				printf("%s\n",name);
				rec_len += curr_dir_entry->rec_len;
				curr_dir_entry = (dir_entry *)(disk + 
					(EXT2_BLOCK_SIZE*(dir_inode->i_block[0]))+(rec_len));
			
			}	
		}
	}else if(dir_inode != NULL && dir_inode->i_mode & EXT2_S_IFREG){
		printf("%s\n",fp);
	}else{
		fprintf(stderr,"No such file or directory\n");
		return ENOENT;
	}


	return 0;
}