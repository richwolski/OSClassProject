#include <stdio.h>
#include <string.h>

main()
{
	char buf[1000];
    int* memStart = (int*)130488;
    int* memEnd = (int*)130548; 
    int* memLoc; 

    for(memLoc = memEnd; memLoc> memStart; memLoc--){
        sprintf(buf, "addr %u is -->%u<--\n", memLoc, *memLoc );
        write(1, buf, strlen(buf));
    }
}
