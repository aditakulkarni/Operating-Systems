#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
	int ret = 1;
	char buff[100000];
	int fd = open("/proc/track_address",O_RDONLY);
	if(fd == -1) {
		printf("\nError opening file..\n");
		exit(1);
	}
	while (ret>0) {
		ret=read(fd,&buff,60);
		if(ret == -1) {
			exit(1);
		}
		printf("\n%s",buff);
	}
	close(fd);
	printf("\n");
	return 0;
}
