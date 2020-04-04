#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
	char name[8];
	int size;
	int blockPointers[8];
	int used;
} INode;

typedef struct {
	char fbl[128];
	INode nodes[16];
} SuperBlock;

int fs_create(char name[], int size) {
	return 0;
}

int fs_delete(char name[]) {
	return 0;
}

int fs_ls() {
	return 0;
}

int fs_read(char name[], int blockNum, char buf[]) {
	return 0;
}

int fs_write(char name[], int blockNum, char buf[]) {
	return 0;
}

int main(int argc, char *argv[]) {

	// Check that we have an argument
	if (argc != 2 ) {
		fprintf(stderr, "usage: %s <diskFileName> \n", argv[0]);
		exit(0);
	}

	// Take filesystem name from args
	char *name = argv[1];

	// Buffer for reading and writing
	char buffer[1024];
	
	// Open filesystem and store descriptor
	int fdesc = open(argv[1], O_RDWR);

	// Parse super block from first 1KB
	SuperBlock sb;

	// Read full super block into buffer
	pread(fdesc, buffer, 1024, 0);
	
	// Scanning position in buffer
	int pos = 0;

	// Extract free block list
	for(int i = 0; i < 128; i++) {
		sb.fbl[i] = buffer[pos++];
	}

	// Extract INodes
	for(int i = 0; i < 16; i++) {

		// Extract name
		for(int j = 0; j < 8; j++) {
			sb.nodes[i].name[j] = buffer[pos++];
		}

		// Extract size
		for(int j = 0; j < 4; j++) {
			sb.nodes[i].size <<= 8;
			sb.nodes[i].size |= buffer[pos++];
		}

		// Extract block pointers
		for(int j = 0; j < 8; j++) {
			for(int k = 0; k < 4; k++) {
				sb.nodes[i].blockPointers[j] <<= 8;
				sb.nodes[i].blockPointers[j] |= buffer[pos++];
			}
		}

		// Extract used boolean
		for(int j = 0; j < 4; j++) {
			sb.nodes[i].used <<= 8;
			sb.nodes[i].used |= buffer[pos++];
		}

	}

	// Actions on the filesystem can now be taken here

	// Close filesystem
	close(fdesc);

	return 0;

}