#include <stdio.h>

main()
{
	int i,j;
	int k = 4;
 	int parentPID = getpid();
	for(j = 0; j < k; j++){
		i = fork();
		printf("I am pid %d, I got %d from fork\n", getpid(),i);
		if(i == 0){
  			printf("I am the child, the process: %i\n", getpid());
  			break;
  	  	}
	}
	if(getpid() == parentPID){
		printf("I am the parent, the process: %i\n", getpid());
		for(j = 0; j < k; j++){
			wait();
		}
	}
}