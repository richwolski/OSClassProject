#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv){
	FILE* a; 
	int n; 
	char buff1[300];
	a = fopen(argv[1], "r");
	fgets(buff1, 300, a);
	char* token = strtok(buff1, " "); 
	
	while(token != NULL){
		n = atoi(token);
		printf("%d\n", n);	
		token = strtok(NULL, " ");
	}

	return 0;

}