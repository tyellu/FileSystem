#include "ext2.h"

unsigned char *disk;

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

	super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);

	group_desc *gd = ((struct ext2_group_desc *)(disk + (GD_BLOCK_INDEX*EXT2_BLOCK_SIZE)));

	unsigned char *block_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_block_bitmap));

	unsigned char *inode_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_inode_bitmap));

	//get unreserved block index and reserve it
	int block_index = get_unreserved_bit(block_bm, (sb->s_blocks_count / 8));
	flip_bit(block_bm,(sb->s_blocks_count / 8), block_index+1);
	//get unreserved inode index and reserve it
	int inode_index = get_unreserved_bit(inode_bm, (sb->s_blocks_count / 32));
	flip_bit(inode_bm,(sb->s_blocks_count / 32), inode_index+1);


	//get the inode struct corresponding to inode_index
	inode *new_dir_inode = (inode *)(disk + (EXT2_BLOCK_SIZE*INODE_TBL_BLOCK) + (INODE_STRUCT_SIZE*inode_index));
	new_dir_inode->i_mode = EXT2_FT_DIR;
	new_dir_inode->i_size = EXT2_BLOCK_SIZE;
	new_dir_inode->i_block[0] = block_index;
	new_dir_inode->i_links_count = 1;
	new_dir_inode->i_blocks = 2;

	char dir_name[256];
	char *path = filepath;
	split(path, dir_name);

	inode *parent_inode = traverse_path(path, disk);

	if(parent_inode != NULL){
		if(parent_inode->i_mode & EXT2_S_IFDIR){
			if(file_exists(disk, parent_inode, dir_name) != NULL){
				return EEXIST;
			}else{
				dir_entry *curr_dir_entry = (dir_entry *)(disk + 
					(EXT2_BLOCK_SIZE*(parent_inode->i_block[0])));
				int curr_size = curr_dir_entry->rec_len;
				while(curr_size < EXT2_BLOCK_SIZE && curr_size > 0){
					if((curr_size + curr_dir_entry->rec_len) == EXT2_BLOCK_SIZE){
						printf("%s\n",curr_dir_entry->name);
						break;
					}else{
						curr_dir_entry = (dir_entry *)(disk + 
							(EXT2_BLOCK_SIZE*(parent_inode->i_block[0])) + curr_size);
						curr_size += curr_dir_entry->rec_len;
					}			
				}
				int req_reclen = EXT2_DIR_ENTRY_SIZE + align(curr_dir_entry->name_len);
				if((curr_dir_entry->rec_len - req_reclen) > 0){
					dir_entry *new_dir_entry = (dir_entry *)(disk + 
					(EXT2_BLOCK_SIZE*(parent_inode->i_block[0])) + req_reclen);
					new_dir_entry->inode = inode_index;
					new_dir_entry->rec_len = (curr_dir_entry->rec_len - req_reclen);
					new_dir_entry->name_len = strlen(dir_name);
					strncpy(new_dir_entry->name, dir_name, strlen(dir_name));
				}else{
				}
			}
		}else if(parent_inode != NULL && parent_inode->i_mode & EXT2_S_IFREG){
			char parent_name[256];
			char *ppath = path ;
			split(ppath, parent_name);
			printf("%s is not a valid directory\n",parent_name);
		}
	}else{
		fprintf(stderr,"No such file or directory\n");
		return ENOENT;
	}

	return 0;
}