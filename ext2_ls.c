#include "ext2.h"
#include "ext2.h"

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

	printf("%d\n",a_flag);
	traverse_path(filepath, disk);


	return 0;
}