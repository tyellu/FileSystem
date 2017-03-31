#include "ext2.h"

int main(int argc, char *argv[])
{
	
	char *filepath = argv[2];

	if (argc < 3) {
        fprintf(stderr,"To run the program ./ext2_mkdir <image file name> <filepath> \n");
        return 1;
    }

    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }

    //get the superblock, group_desc block, inode_bitmap, block_bitmap
	super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
	group_desc *gd = ((struct ext2_group_desc *)(disk + (GD_BLOCK_INDEX*EXT2_BLOCK_SIZE)));
	unsigned char *block_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_block_bitmap));
	unsigned char *inode_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_inode_bitmap));

	//get unreserved block index and reserve it
	int block_index = get_unreserved_bit(block_bm, (sb->s_blocks_count / 8));
	if(block_index == -1){
		fprintf(stderr, "Disk out of memory\n");
		return 0;
	}
	flip_bit(block_bm,(sb->s_blocks_count / 8), block_index);
	// printbm(block_bm, (sb->s_blocks_count / 8));
	//get unreserved inode index and reserve it
	int inode_index = get_unreserved_bit(inode_bm, (sb->s_blocks_count / 32));
	if(inode_index == -1){
		fprintf(stderr, "Disk out of memory\n");
		return 0;
	}
	flip_bit(inode_bm,(sb->s_blocks_count / 32), inode_index);
	// printbm(inode_bm, (sb->s_blocks_count / 32));

	//update group group_desc
	gd->bg_free_blocks_count -= 1;
	gd->bg_free_inodes_count -= 1;


	//get the inode struct corresponding to inode_index
	inode *new_dir_inode = (inode *)(disk + (EXT2_BLOCK_SIZE*INODE_TBL_BLOCK) + (INODE_STRUCT_SIZE*inode_index));
	new_dir_inode->i_mode = EXT2_S_IFDIR;
	new_dir_inode->i_size = EXT2_BLOCK_SIZE;
	new_dir_inode->i_block[0] = (block_index);
	new_dir_inode->i_links_count = 1;
	new_dir_inode->i_blocks = 2;
	new_dir_inode-> i_dtime = 0;

	//get dir_name and dirpath
	char dir_name[256];
	char *path = filepath;
	split(path, dir_name);

	inode *parent_inode = traverse_path(path, disk);

	if(parent_inode != NULL){
		if(parent_inode->i_mode & EXT2_S_IFDIR){
			if(file_exists(disk, parent_inode, dir_name) != NULL){
				flip_bit(inode_bm,(sb->s_blocks_count / 32), inode_index);
                flip_bit(block_bm,(sb->s_blocks_count / 8), block_index);
				fprintf(stderr, "Directory with the name %s, already exists\n", dir_name);
				return EEXIST;
			}else{
				dir_entry *curr_dir_entry;
				int i;
				for(i=0; ((i < (parent_inode->i_blocks / 2)) && (i < 11)); i++){
					curr_dir_entry = (dir_entry *)(disk + 
						(EXT2_BLOCK_SIZE*(parent_inode->i_block[i])));
				}
				int curr_size = curr_dir_entry->rec_len;
				int prev_size = 0;
				int block;
				for(block=0; ((block < (parent_inode->i_blocks / 2)) && (block < 11)); block++){
					while(curr_size < EXT2_BLOCK_SIZE && curr_size > 0){
						curr_dir_entry = (dir_entry *)(disk + 
							(EXT2_BLOCK_SIZE*(parent_inode->i_block[block])) + curr_size);
							prev_size = curr_size;
							curr_size += curr_dir_entry->rec_len;		
					}
				}
				int curr_dir_reclen = EXT2_DIR_ENTRY_SIZE + align(curr_dir_entry->name_len);
				int req_reclen = EXT2_DIR_ENTRY_SIZE + align(strlen(dir_name));
				if(req_reclen < curr_dir_entry->rec_len){
					curr_dir_entry->rec_len = (unsigned short) curr_dir_reclen;
					dir_entry *new_dir_entry = (dir_entry *)(disk + 
						(EXT2_BLOCK_SIZE*(parent_inode->i_block[block-1]))+ (prev_size+curr_dir_reclen));
					new_dir_entry->inode = (inode_index+1);
					new_dir_entry->rec_len = (unsigned short)(EXT2_BLOCK_SIZE - (prev_size+curr_dir_reclen));
					new_dir_entry->name_len = strlen(dir_name);
					new_dir_entry->file_type = EXT2_FT_DIR;
					strncpy(new_dir_entry->name, dir_name, strlen(dir_name));
				}else{
					//extend the data block
					int block_index = get_unreserved_bit(block_bm, (sb->s_blocks_count / 8));
					if(block_index == -1){
						fprintf(stderr, "Disk out of memory\n");
						return 0;
					}
					flip_bit(block_bm,(sb->s_blocks_count / 8), block_index);
					gd->bg_free_blocks_count -= 1;
					new_dir_inode->i_block[block] = block_index;
					new_dir_inode->i_blocks += 2;
					dir_entry *new_dir_entry = (dir_entry *)(disk + 
						(EXT2_BLOCK_SIZE*(parent_inode->i_block[block]))+ (prev_size+curr_dir_reclen));
					new_dir_entry->inode = (inode_index+1);
					new_dir_entry->rec_len = (unsigned short)(EXT2_BLOCK_SIZE);
					new_dir_entry->name_len = strlen(dir_name);
					new_dir_entry->file_type = EXT2_FT_DIR;
					strncpy(new_dir_entry->name, dir_name, strlen(dir_name));
				}

			}
		}else if(parent_inode != NULL && parent_inode->i_mode & EXT2_S_IFREG){
			char parent_name[256];
			char *ppath = path ;
			split(ppath, parent_name);
			fprintf(stderr,"%s is not a valid directory\n",parent_name);
			return ENOENT;
		}
	}else{
		fprintf(stderr,"No such file or directory\n");
		return ENOENT;
	}

	dir_entry *curr_entry = (dir_entry *)(disk + (EXT2_BLOCK_SIZE*block_index));
	curr_entry->inode = (inode_index+1);
	curr_entry->rec_len = 12;
	curr_entry->name_len = strlen(".");
	curr_entry->file_type = EXT2_FT_DIR;
	curr_entry->name[0] = '.';

	//second entry
	curr_entry = (dir_entry *)(disk + (EXT2_BLOCK_SIZE*block_index) + 12);
	curr_entry->rec_len = 1012;
	curr_entry->name_len = strlen("..");
	curr_entry->file_type = EXT2_FT_DIR;
	curr_entry->name[0] = '.';
	curr_entry->name[1] = '.';

	if(strcmp(path, "/") == 0){
		curr_entry->inode = 2;
	}else{
		char pp_name[256];
		char *ppath = path ;
		split(ppath, pp_name);
		inode *pparent = traverse_path(ppath, disk);
		dir_entry *pentry = file_exists(disk, pparent, pp_name);
		curr_entry->inode = pentry->inode;
	}

	return 0;
}
