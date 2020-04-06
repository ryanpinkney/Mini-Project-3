#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main(int argc, char *argv[]) {

	// Variable declaration for input arguments
	int n;
	int m;
	char fname[100];
	
	// Section to input the name of the file that will be read
	printf("what file name are you using: ");
	scanf("%s", fname);
	FILE *fptr = fopen(fname, "r");

	// Scan file for n and m
	fscanf(fptr, "%d", &n);
	fscanf(fptr, "%d", &m);

	// Outputs the first two values to make it easy to check for accuracy
	printf("n=%d \n", n);
	printf("m=%d \n", m);

	int address;
	
	// The while loop designed to find and output page numbers and offsets
	while(fscanf(fptr, "%d", &address) != EOF){

		// Page is stored in high-order bits
		int page = address >> n;

		// Offset is stored in low-order bits
		int offset = address & (0xffff >> m);

		printf("Virtual address %i is in page number %i and offset %i \n", address, page, offset);
		
	}

}