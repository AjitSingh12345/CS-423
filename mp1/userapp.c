#include "userapp.h"
#include <unistd.h> 
#include <stdio.h> 
#include <sys/types.h>
#include <stdlib.h>

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

	// sleep 15 sec
	printf("start sleep \n");
	sleep(15);
	printf("done sleep \n");

	// FILE *fp = popen(command, "w");
	// fprintf(fp, "cat /proc/mp1/status");
	// fclose(fp);
	
	system("cat /proc/mp1/status");
	return 0;
}
