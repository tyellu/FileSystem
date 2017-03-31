#include "ext2.h"

int main(int argc, char *argv[])
{
	
	bool s_flag = false;


	if !((argc == 3) || (argc == 4)) {
        fprintf(stderr,"To run the program ./ext2_ln <image file name> <source filepath> <disk filepath> \n");
        return 1;
    }

    char *srcfilepath = argv[2];
	char *diskfilepath = argv[3];

    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }

	while((option = getopt(argc, argv, "s:")) != -1){
	    switch (option){
	    case 's':
	    	s_flag = true;
			srcfilepath = argv[3];
			diskfilepath = argv[4];
	    	break;
	    default:
	    	break;
	    }
	}


	super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);

	group_desc *gd = ((struct ext2_group_desc *)(disk + (GD_BLOCK_INDEX*EXT2_BLOCK_SIZE)));

	unsigned char *block_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_block_bitmap));

	unsigned char *inode_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_inode_bitmap));

	if (s_flag) {
		//get unreserved inode index and reserve it
		int free_inode_index = get_unreserved_bit(inode_bm, (sb->s_blocks_count / 32));
		if(free_inode_index == -1){
			fprintf(stderr, "Disk out of memory\n");
			exit(0);
		}

		flip_bit(inode_bm,(sb->s_blocks_count / 32), free_inode_index);

	}

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
				//when link is a symlink
				if (lnk_parent_inode->i_mode & EXT2_S_IFDIR) {
					//when the destination of the link is a directory
					if (file_exists(disk, lnk_parent_inode, link_name) != NULL) {
						//error since the link location already has file by given name
						fprintf(stderr, "File %s in link location already exists\n", link_name);
						return EEXIST;

					} else {
						//creates a symlink type directory entry 
						dir_entry *lnk_dir_entry;
						int i;
						for (i=0; ((i < (lnk_parent_inode / 2)) && (i < 11)); i++) {
							lnk_dir_entry = (dir_entry *)(disk +
									(EXT2_BLOCK_SIZE*(lnk_parent_inode->i_block[i])));
						}
						int lnk_size = lnk_dir_entry->rec_len;
						int prev_size = 0;
						int block;
						for (block=0; ((block < (lnk_parent_inode->i_blocks / 2)) && (block < 11)); block++) {
							while ((lnk_size < EXT2_BLOCK_SIZE) && (lnk_size > 0)) {
								lnk_dir_entry = (dir_entry *)(disk +
										(EXT2_BLOCK_SIZE*(lnk_parent_inode->i_block[block])) + lnk_size);
								prev_size = lnk_size;
								lnk_size += lnk_dir_entry->rec_len;
							}
						}

						int lnk_dir_reclen = EXT2_DIR_ENTRY_SIZE + align(lnk_dir_entry->name_len);
						int needed_reclen = EXT2_DIR_ENTRY_SIZE + align(strlen(link_name));
						if(needed_reclen < lnk_dir_entry->rec_len) {
							lnk_dir_entry->rec_len = (unsigned short) lnk_dir_reclen;
							dir_entry *new_dir_entry = (dir_entry *)(disk +
								(EXT2_BLOCK_SIZE*(lnk_parent_inode->i_block[block-1])) + (prev_size + lnk_dir_reclen));
							new_dir_entry->inode = (inode_index+1);
							new_dir_entry->rec_len = (unsigned short)(EXT2_BLOCK_SIZE - (prev_size + lnk_dir_reclen));
							new_dir_entry->name_len = strlen(link_name);
							new_dir_entry->file_type = EXT2_FT_SYMLINK;
							strncpy(new_dir_entry->name, link_name, strlen(link_name));
						}
					}

					//write the filepath into a datablock
					int *id_block = (int *)(disk + (EXT2_BLOCK_SIZE*new_link_inode->i_block[12]));
					
					//get unreserved block index and reserve it
					int free_block_index = get_unreserved_bit(block_bm, (sb->s_blocks_count / 8));
					if(free_block_index == -1){
						fprintf(stderr, "Disk out of memory\n");
						exit(0);
					}
					flip_bit(block_bm,(sb->s_blocks_count / 8), free_block_index);

					new_link_inode->i_block[i] = free_block_index;
					memcpy((disk + EXT2_BLOCK_SIZE*free_block_index), srcfilepath, strlen(srcfilepath));


				} else if (lnk_parent_inode != NULL && lnk_parent_inode->i_mode & EXT2_S_IFREG) {
					//when the destination of the link is a file
					char parent_name[256];
					char *ppath = linkpath;
					split(ppath, parent_name);
					fprintf(stderr, "%s is not a valid directory\n", parent_name);
					return EISDIR;
				}


			} else {
				//when link is a hard link
				if (src_parent_inode->i_mode & EXT2_S_IFDIR) {
					//when parent directory of source file is a directory
					if (file_exists(disk, src_parent_inode, src_name) != NULL){
						//collects the necessary info of the file if it exists
						dir_entry *src_dir_entry = file_exists(disk, src_parent_inode, src_name);
						inode *src_inode = traverse_path(srcfilepath, disk);
						unsigned int src_inode_value = src_dir_entry->inode;
					} else {
						//error for when file to be linked is not in directory
						fprintf(stderr, "File to be linked to deos not exist\n");
						return ENOENT;
					}

				} else if (src_parent_inode != NULL && src_parent_inode->i_mode & EXT2_S_IFREG) {
					//when parent directory of source file is a file
					char parent_name1[256];
					char *ppath1 = srcpath ;
					split(ppath1, parent_name1);
					fprintf(stderr, "%s is not a valid directory\n", parent_name1);
					return EISDIR;
				}

				if (lnk_parent_inode->i_mode & EXT2_S_IFDIR) {
					//when the destination of the link is a directory
					if (file_exists(disk, lnk_parent_inode, link_name) != NULL) {
						//error since the link location already has file by given name
						fprintf(stderr, "File %s in link location already exists\n", link_name);
						return EEXIST;
					} else {
						//creates a file with a hard link to the specified file's inode
						dir_entry *lnk_dir_entry;
						int i;
						for (i=0; ((i < (lnk_parent_inode / 2)) && (i < 11)); i++) {
							lnk_dir_entry = (dir_entry *)(disk +
									(EXT2_BLOCK_SIZE*(lnk_parent_inode->i_block[i])));
						}
						int lnk_size = lnk_dir_entry->rec_len;
						int prev_size = 0;
						int block;
						for (block=0; ((block < (lnk_parent_inode->i_blocks / 2)) && (block < 11)); block++) {
							while ((lnk_size < EXT2_BLOCK_SIZE) && (lnk_size > 0)) {
								lnk_dir_entry = (dir_entry *)(disk +
										(EXT2_BLOCK_SIZE*(lnk_parent_inode->i_block[block])) + lnk_size);
								prev_size = lnk_size;
								lnk_size += lnk_dir_entry->rec_len;
							}
						}
						int lnk_dir_reclen = EXT2_DIR_ENTRY_SIZE + align(lnk_dir_entry->name_len);
						int needed_reclen = EXT2_DIR_ENTRY_SIZE + align(strlen(link_name));
						if(needed_reclen < lnk_dir_entry->rec_len) {
							lnk_dir_entry->rec_len = (unsigned short) lnk_dir_reclen;
							dir_entry *new_dir_entry = (dir_entry *)(disk +
								(EXT2_BLOCK_SIZE*(lnk_parent_inode->i_block[block-1])) + (prev_size + lnk_dir_reclen));
							new_dir_entry->inode = src_inode_value;
							src_inode->i_links_count += 1;
							new_dir_entry->rec_len = (unsigned short)(EXT2_BLOCK_SIZE - (prev_size + lnk_dir_reclen));
							new_dir_entry->name_len = strlen(link_name);
							new_dir_entry->file_type = EXT2_FT_REG_FILE;
							strncpy(new_dir_entry->name, link_name, strlen(link_name));
						}
					}

				} else if (lnk_parent_inode != NULL && lnk_parent_inode->i_mode & EXT2_S_IFREG) {
					//when the destination of the link is a file
					char parent_name2[256];
					char *ppath2 = linkpath;
					split(ppath2, parent_name2);
					fprintf(stderr, "%s is not a valid directory\n", parent_name2);
					return EISDIR;
				}

			}
		}
		else {
			fprintf(stderr, "Destination folder for link does not exist.\n")
			return 0;
		}
	} else {
		fprintf(stderr, "File to be linked to does not exist.\n");
		return 0;
	}
