#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <string.h> 

void copy_read_write(int fd_from, int fd_to);
void copy_mmap(int fd_from, int fd_to);
void help_information();

int main(int argc, char **argv) //argc is the number of arguments,and argv is parameters
{
	int fd_from, fd_to;
	char arg; 
	int mmap = 0;
    arg = getopt(argc, argv, "mh");
    // Check argument number
    switch (argc)
    {
    case 3: //copy <file_name> <new_file_name>
            printf("copy <file_name> <new_file_name>\n");
			break;
    case 2:  //
        if(arg=='h'){//copy [-h]
			help_information();
			return 0;
		}else{
			printf("incorrect syntax \n");
			return 1;
		}	
    case 4: // copy [-m] <file_name> <new_file_name>
	    if(arg=='m'){
			mmap=1;
			break;
		}else {
            printf("incorrect syntax\n");
			return 1;
        }
    default: // other
        printf("Too many or too few input arguments\n");
        return 1;
    }
	
	fd_from = open(argv[optind], O_RDONLY);
	fd_to = open(argv[optind + 1], O_RDWR | O_CREAT, 0666);//0666=-rw-rw-rw  permission can read write
	
	char *filefrom = argv[optind]; //file name
	char *fileto = argv[optind+1] ; 
	if (fd_from < 0) { 
	  fprintf(stderr, "ERROR in reading %s \n", argv[optind]);
	  return 1;
	}
	if (fd_to<0) {
	  fprintf(stderr, "ERROR in writing %s\n",argv[optind + 1]);
	  return 1;
	}
	if (strcmp(filefrom, fileto) == 0)//compare with filefrom and fileto to check if these two files conflict
	{//if conflicts
		printf("source file and destination file cannot be the same\n");
		return 1;
	}
	
	if (mmap) { //mmap
	 copy_mmap(fd_from, fd_to);
	 printf("Succeeded in copying '%s' to '%s' by using mmap() and memcpy() functions.\n",filefrom, fileto);													
	 
	} else {
	  copy_read_write(fd_from, fd_to);
	  printf("Succeeded in copying '%s' to '%s' by using read() and write() functions.\n",filefrom, fileto);
	}
	if (close(fd_from)){ //close function will return 0 if success
	  fprintf(stderr, "ERROR: Can not close %s!\n", filefrom);
	}
	if (close(fd_to)){ //close destination file.
	  fprintf(stderr, "ERROR: Can not close %s!\n", fileto);
	}
	return 0;
}

void copy_read_write(int fd_from, int fd_to)//function read write
{
	int ret;
	char buf[1024];//buffer size
	while ((ret = read(fd_from, buf, sizeof(buf))) != 0)//If read is successful, it returns the number of bytes read, and if it has reached the end, it returns 0, 
	{						
		if (ret == -1) err(1, "reading source file");//error Returns -1
		if ((write(fd_to, buf, ret)) == -1) err(1, "writing to destination file");
	}
}

void copy_mmap(int fd_from, int fd_to)
{
	 struct stat sb;
    char *fdes,*fsrc;
	//Fstat () is used to copy the file state indicated by the parameter fd_from (file descriptor) to the structure (struct STAT) indicated by the parameter sb.
    if (fstat(fd_from, &sb) == -1){ 
        fprintf(stderr,"ERROR  in fstat of the source file!\n");
        return;
    } 
    /*ftruncate function changes the file size of the file(fd_to descriptor) 
	to the size sb.st_size,  if the original file size is greater than
	the parameter length,the excess will be deleted
	*/
	ftruncate(fd_to,sb.st_size);
	/*
	  mmap function is used to map a file to the memory.
	  void *mmap(void *start,size_t length,int prot,int flags,int fd,off_t offsize); 
	  arguments:
	  start：
    	    it points to the starting address of the memory to be mapped.It is usually set to null,
	        which means that the system will automatically select the address.
	        After the mapping is successfu,the address will be returned.
	  prot：
	        It represents the protection mode of the mapping area.
			For example:
			  PROT_READ:The read mapping area can be read.
	  MAP_SHARED:
	            the data written to the mapping area is copied back
				to the file and is shared by other processes that map the file.
	*/
	fsrc = mmap(NULL, sb.st_size, PROT_READ , MAP_SHARED, fd_from, 0);
	fdes   = mmap(NULL, sb.st_size, PROT_WRITE, MAP_SHARED, fd_to, 0);
//MAP_FAILED represents the error return
	if (fsrc == MAP_FAILED) err(1, "Error in mapping source file");
	if (fdes   == MAP_FAILED) err(1, "Error in mapping destination file");
//Copy the contents of the memory by mapping address
	memcpy(fdes, fsrc, sb.st_size);
	//release the mapping relationship
	if(munmap(fsrc, sb.st_size)){
	fprintf(stderr,"munmap source file destination fail!\n");
    }
	if(munmap(fdes, sb.st_size)){
	fprintf(stderr,"munmap destination file destination fail!\n");
    }
	
}

void help_information()
{
    printf("Help:\n\
    syntax: \n\
        copy [-m] <file_name> <new_file_name> \n\
        copy <file_name> <new_file_name>\n\
        copy [-h]\n\
    -h: help\n\
    -m: using mmap and memcpy\n\
    ");   
}
