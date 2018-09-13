#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
	int ret = 1;
	char buff[1000];
	int fd = open("/dev/process_list",O_RDONLY);
	if(fd == -1) {
		printf("\nError opening file..\n");
		exit(1);
	}
	while (ret>0) {
		ret=read(fd,&buff,1000);
		if(ret == -1) {
			printf("\nError in reading..\n");
			exit(1);
		}
		printf("%s",buff);
	}
	close(fd);
	printf("\n");
	return 0;
}
