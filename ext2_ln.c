#include "ext2.h"

int main(int argc, char *argv[])
{
	
	bool s_flag = false;
	char *srcfilepath = argv[2];
	char *diskfilepath = argv[3];

	if (argc < 4) {
        fprintf(stderr,"To run the program ./ext2_ln <image file name> <source filepath> <disk filepath> \n");
        return 1;
    }

    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }

	while((option = getopt(argc, argv, "s")) != -1){
	    switch (option){
	    case 's':
	    	s_flag = true;
	    	break;
	    default:
	    	break;
	    }
	}


	super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);

	group_desc *gd = ((struct ext2_group_desc *)(disk + (GD_BLOCK_INDEX*EXT2_BLOCK_SIZE)));

	unsigned char *block_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_block_bitmap));

	unsigned char *inode_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_inode_bitmap));

	//get unreserved block index and reserve it
	int free_block_index = get_unreserved_bit(block_bm, (sb->s_blocks_count / 8));
	if(free_block_index == -1){
		fprintf(stderr, "Disk out of memory\n");
		return 0;
	}

	flip_bit(block_bm,(sb->s_blocks_count / 8), free_block_index);
	
	//get unreserved inode index and reserve it
	int free_inode_index = get_unreserved_bit(inode_bm, (sb->s_blocks_count / 32));
	if(free_inode_index == -1){
		fprintf(stderr, "Disk out of memory\n");
		return 0;
	}

	flip_bit(inode_bm,(sb->s_blocks_count / 32), free_inode_index);

	//get the inode struct corresponding to inode_index
	inode *new_link_inode = (inode *)(disk + (EXT2_BLOCK_SIZE*INODE_TBL_BLOCK) + (INODE_STRUCT_SIZE*free_inode_index));
	new_link_inode->i_mode = EXT2_S_IFLNK;
	new_link_inode->i_size = EXT2_BLOCK_SIZE;
	new_link_inode->i_block[0] = (block_index);
	new_link_inode->i_links_count = 1;
	new_link_inode->i_blocks = 2;
	new_link_inode-> i_dtime = 0;

	//get the name of the file to be linked to and path to it
	char src_name[256];
	char *srcpath = srcfilepath;
	split(srcpath, src_name);

	//get the link file name and the path to it
	char link_name[256];
	char *linkpath = diskfilepath;
	split(linkpath, link_name);

	inode *src_parent_inode = traverse_path(srcpath, disk);
	inode *lnk_parent_inode = traverse_path(linkpath, disk);

	if (src_parent_inode != NULL) {
		if (lnk_parent_inode != NULL) {
			if (s_flag) {

			} else {
				if (src_parent_inode->i_mode & EXT2_S_IFDIR) {
					if (file_exists(disk, src_parent_inode, src_name) != NULL){
						dir_entry *src_dir_entry = file_exists(disk, src_parent_inode, src_name);
						unsigned int src_inode = src_dir_entry->inode;
					} else {
						fprintf(stderr, "File to be linked to does not exist\n");
					}

				} else if (src_parent_inode != NULL && src_parent_inode->i_mode & EXT2_S_IFREG){
					char parent_name[256];
					char *ppath = path ;
					split(ppath, parent_name);
					printf("%s is not a valid directory\n",parent_name);
				}
			}
		}
		else {
			fprintf(stderr, "Destination folder for link does not exist.\n")	 
		}
	} else {
		fprintf(stderr, "File to be linked to does not exist.\n");
	}
