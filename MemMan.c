#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main() {

	int n;
	int m;
	char fname[100];
	
	//section to input the name of the file that will be read
	printf("what file name are you using: ");
	scanf("%s", fname);
	FILE *fptr = fopen(fname, "r");

	fscanf(fptr, "%d", &n);
	fscanf(fptr, "%d", &m);

	//outputs the first two values to make it easy to check for accuracy
	printf("n=%d \n", n);
	printf("m=%d \n", m);

	int address;
	
	//the while loop designed to find and output page numbers and offsets
	while(fscanf(fptr, "%d", &address) != EOF){

		//important data is stored here
		int page = address >> n;
		int offset = address & (0xffff >> m);

		printf("Virtual address %i is in page number %i and offset %i \n", address, page, offset);
		
	}

}