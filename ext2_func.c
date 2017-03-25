#include "ext2.h"
#define maxchar 1000


bool valid_path(char fp){
	if (fp == '/'){
		return true;
	}
	return false;
}


void traverse_path(char *filepath, unsigned char *disk){
	char fp[maxchar];
	strncpy(fp, filepath, strlen(filepath)); 
	if(valid_path(fp[0])){
		char *pch;
		pch = strtok (filepath,"/");
		while (pch != NULL)
		{
			printf ("%s\n",pch);
		    pch = strtok (NULL, "/");
		}
	}
}


