
#define delayCycles 100000000

#include <stdio.h>

main()
{
	char ch;
	int n;
	int stalling;
	int blah;

	char buffer[1000];
	blah = -1;
	for(stalling = 0; stalling < 10* delayCycles; stalling++)
	{
		blah = 2*blah +3;
		if( !(stalling % delayCycles))
			write(1,"stalling...\n",13);
	}
		
	n = read(0, buffer, 995);
	write(1, buffer, n);
	
	if (n < 0) {
		perror("cat");
	}
}
