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

	struct ext2_super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);

	struct ext2_group_desc *gd = ((struct ext2_group_desc *)(disk + (GD_BLOCK_INDEX*EXT2_BLOCK_SIZE)));

	unsigned char *block_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_block_bitmap));

	unsigned char *inode_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_inode_bitmap));

	//get unreserved block index
	int block_index = get_unreserved_bit(block_bm, (sb->s_blocks_count / 8));

	//get unreserved inode index
	int inode_index = get_unreserved_bit(inode_bm, (sb->s_blocks_count / 32));


	//get the inode struct corresponding to inode_index
	inode *new_dir_inode = (inode *)(disk + (EXT2_BLOCK_SIZE*INODE_TBL_BLOCK) + (INODE_STRUCT_SIZE*inode_index));
	new_dir_inode->i_mode = EXT2_FT_DIR;
	new_dir_inode->i_size = EXT2_BLOCK_SIZE;
	new_dir_inode->i_block[0] = block_index;
	new_dir_inode->i_links_count = 1;
	new_dir_inode->i_blocks = 2;

	char file_name[256];
	char *path = filepath;
	split(path, file_name);

	inode *parent_inode = traverse_path(path, disk);

	if(parent_inode != NULL){
		if(parent_inode->i_mode & EXT2_S_IFDIR){
			if(file_exists(disk, parent_inode, file_name)){
				return EEXIST;
			}else{

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