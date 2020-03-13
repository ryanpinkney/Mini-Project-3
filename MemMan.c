#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main() {

int n = 0;
int m = 0;
char fname[100];
//section to input the name of the file that will be read
printf("what file name are you using: ");
scanf("%s", fname);
FILE *fptr = fopen(fname, "r");

fscanf(fptr, "%d", &n);
fscanf(fptr, "%d", &m);
//outputs the first two values to make it easy to check for accuracy
printf("%d \n", n);
printf("%d \n", m);

long address;
//the while loop designed to find and output page numbers and offsets
while(fscanf(fptr, "%ld", &address) != EOF){
	
	//important data is stored here
	long utility = address;
	long page = 0;
	long offset = 0;
	
	printf("utility is now %ld \n", utility);

	//the offset is calculated and stored
	for(int n2 = 0; n2 < n; n2++){

		//the offset is incremented by the remainder
		int rem = utility % 10;
		rem = rem<<n2;
		offset = offset + rem;
		printf("offset is now %ld \n", offset);
		utility = utility/10;
		printf("utility is now %ld \n", utility);
	}
	//the page is calculated and stored
	for(int m2 = 0; m2 < m; m2++){

		//the page is incremented by the remainder
		int rem = utility % 10;
		rem = rem<<m2;
		page = page + rem;
		printf("page is now %ld \n", page);
		utility = utility/10;
		printf("utility is now %ld \n", utility);
	}


	printf("Virtual address %ld is in page number %ld and offset %ld \n", address, page, offset);
}
}
