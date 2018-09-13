#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int str_is_decimal_num(char * str);

int main(int argc, char *argv[]) {
	int height, children, i, j, return_status;
	char space[5000];

	//Check if there are proper commandline arguments
	if(argc != 3) {
		printf("\nUsage : %s height children\n", argv[0]);
		exit(1);
	}

	//Check if arguments are decimal numbers
	if((!str_is_decimal_num(argv[1])) || (!str_is_decimal_num(argv[2]))) {
		printf("\nArgument must be a decimal number\nUsage : %s height children\n", argv[0]);
		exit(1);
	}

	height = atoi(argv[1]);
	children = atoi(argv[2]);

	char string_height[10];
	sprintf(string_height,"%d",(height-1));

	//For indentation
	memset(space, '\0', 5000);
	memset(space, '\t', height);

	//Display information of process 
	printf("\n\n%s(%d): Process starting\n%s(%d): Parent's id = (%d)\n%s(%d): Height in the tree = (%d)\n%s(%d): Creating (%d) children at height (%d)\n", space, getpid(), space, getpid(), getppid(), space, getpid(), height, space, getpid(), children, height-1);

	pid_t pid;

	if(height > 1) {
		for(i=0; i<children; i++) {
			//Fork a child process
			pid = fork();
			if(pid == 0) {
				//Recursion by exec system call
				return_status = execl(argv[0], argv[0], string_height, argv[2], NULL);
				//Check if exec failed
				if(return_status == -1) {
					printf("\nexec() failed..\n");
					return 1;
				}
			}
			//Check if fork failed
			else if(pid < 0) {
				printf("\nFork failed..\n");
				return 1;
			}
		}			
	}

	//Wait for all children to complete
	for(j=0; j<children; j++) {

		return_status = wait(NULL);

		//Check if wait failed
		if(return_status == -1 && height > 1) {
			printf("\nWait failed..\n");
			return 1;
		}
	}

	printf("\n\n%s(%d): Terminating at height (%d)\n", space, getpid(), height);

	return 0;
}

//Check if string is a decimal number
int str_is_decimal_num(char * str)
{
    int ret = 1;
    char * p = NULL;    
    if (NULL == str)
    {
        printf("error: null input\n");
        return 0;
    }
    p = str;
    while (*p)
    {
        if (*p < '0' || *p > '9')
        {
            ret = 0;
            break;
        }
        p += 1;
    }
    return ret;
}
