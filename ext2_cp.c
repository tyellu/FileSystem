#include "ext2.h"

int main(int argc, char *argv[])
{
	

	if (argc < 4) {
        fprintf(stderr,"To run the program ./ext2_cp <image file name> <native filepath> <disk filepath> \n");
        return 1;
    }

    char *native_path = argv[2];
    char *filepath = argv[3];


    //read disk image
    int fd = open(argv[1], O_RDWR);
    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
        perror("mmap_disk");
        exit(1);
    }

    //get filepath
    char dir_name[256];
    char *path = filepath;
    split(path, dir_name);

    //read file and get size and the calc the blocks required
    int file_fd = open(native_path, O_RDONLY);
    int file_size = lseek(file_fd, 0, SEEK_END);
    int req_blocks = ((file_size - 1) / EXT2_BLOCK_SIZE) + 1;  
    unsigned char* src_file = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
    if (src_file == MAP_FAILED) {
       perror("mmap_file");
       exit(1);
    }    

    //get filename
    char file_name[256];
    char *path2 = native_path;
    split(path2, file_name);

    //get the superblock, group_desc block, inode_bitmap, block_bitmap
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
	struct ext2_group_desc *gd = ((struct ext2_group_desc *)(disk + (GD_BLOCK_INDEX*EXT2_BLOCK_SIZE)));
	unsigned char *block_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_block_bitmap));
	unsigned char *inode_bm = (unsigned char *)(disk + (EXT2_BLOCK_SIZE*gd->bg_inode_bitmap));

    
    //get unreserved inode index and reserve it
    int inode_index = get_unreserved_bit(inode_bm, (sb->s_blocks_count / 32));
    if(inode_index == -1){
        fprintf(stderr, "Disk out of memory\n");
        return 0;
    }
    flip_bit(inode_bm,(sb->s_blocks_count / 32), inode_index);
    // printbm(inode_bm, (sb->s_blocks_count / 32));
    gd->bg_free_inodes_count -= 1;

    //get the inode struct corresponding to inode_index
    inode *new_file_inode = (inode *)(disk + (EXT2_BLOCK_SIZE*INODE_TBL_BLOCK) + (INODE_STRUCT_SIZE*inode_index));
    new_file_inode->i_mode = EXT2_S_IFREG;
    new_file_inode->i_size = file_size;
    // new_dir_inode->i_block[0] = (block_index);
    new_file_inode->i_links_count = 1;
    new_file_inode->i_blocks = req_blocks*2;
    new_file_inode-> i_dtime = 0;
    if(req_blocks > 11){
        int block_index = get_unreserved_bit(block_bm, (sb->s_blocks_count / 8));
        if(block_index == -1){
            fprintf(stderr, "Disk out of memory\n");
            exit(0);
        }
        flip_bit(block_bm,(sb->s_blocks_count / 8), block_index);
        new_file_inode->i_block[12] = block_index;
    }

    //create a dir_entry in the parent
    inode *parent_inode = traverse_path(path, disk);
    if(parent_inode != NULL){
        if(parent_inode->i_mode & EXT2_S_IFDIR){
            if(file_exists(disk, parent_inode,  file_name) != NULL){
                fprintf(stderr, "File with the name %s, already exists\n", file_name);
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
                int req_reclen = EXT2_DIR_ENTRY_SIZE + align(strlen(file_name));
                if(req_reclen < curr_dir_entry->rec_len){
                    curr_dir_entry->rec_len = (unsigned short) curr_dir_reclen;
                    dir_entry *new_dir_entry = (dir_entry *)(disk + 
                        (EXT2_BLOCK_SIZE*(parent_inode->i_block[block-1]))+ (prev_size+curr_dir_reclen));
                    new_dir_entry->inode = (inode_index+1);
                    new_dir_entry->rec_len = (unsigned short)(EXT2_BLOCK_SIZE - (prev_size+curr_dir_reclen));
                    new_dir_entry->name_len = strlen(file_name);
                    new_dir_entry->file_type = EXT2_FT_REG_FILE;
                    strncpy(new_dir_entry->name, file_name, strlen(file_name));
                }else{
                    //extend the data block
                    int block_index = get_unreserved_bit(block_bm, (sb->s_blocks_count / 8));
                    if(block_index == -1){
                        fprintf(stderr, "Disk out of memory\n");
                        return 0;
                    }
                    flip_bit(block_bm,(sb->s_blocks_count / 8), block_index);
                    gd->bg_free_blocks_count -= 1;
                    new_file_inode->i_block[block] = block_index;
                    new_file_inode->i_blocks += 2;
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
        }
    }else{
        fprintf(stderr,"No such file or directory\n");
        return ENOENT;
    }

    int id_counter = 0;
    int cp_counter = 0;
    int *id_block = (int *)(disk + (EXT2_BLOCK_SIZE*new_file_inode->i_block[12]));
    //copy the data from the file over to the disk
    int i;
    for(i = 0; i < req_blocks; i++){
        int block_index = get_unreserved_bit(block_bm, (sb->s_blocks_count / 8));
        if(block_index == -1){
            fprintf(stderr, "Disk out of memory\n");
            exit(0);
        }
        flip_bit(block_bm,(sb->s_blocks_count / 8), block_index);
        gd->bg_free_blocks_count -= 1;
        if(i < 11){
            new_file_inode->i_block[i]= block_index;
        }else{
            if(id_counter < (EXT2_BLOCK_SIZE / 4)){
                id_block[id_counter] = block_index;
                id_counter++; 
                
            }
        }
        
        if((file_size - cp_counter) > 1024){
            memcpy((disk + EXT2_BLOCK_SIZE * block_index), (src_file + (EXT2_BLOCK_SIZE * i)), EXT2_BLOCK_SIZE);
            cp_counter += 1024;
        } else {
            memcpy((disk + EXT2_BLOCK_SIZE * block_index), (src_file + (EXT2_BLOCK_SIZE * i)), (file_size - cp_counter - 1));
        }
    }

    return 0;

}
