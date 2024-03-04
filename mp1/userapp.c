#include "userapp.h"
#include <unistd.h> 
#include <stdio.h> 
#include <sys/types.h>

int main(int argc, char* argv[])
{
	int pid = getpid();
	FILE *fptr = fopen("/proc/mp1/status", "w");
	if (fptr == NULL) {
		printf("could not open file :(");
		return 0;
	} 

	fprintf(fptr, "%d", pid);
	fclose(fptr);

	return 0;
}
