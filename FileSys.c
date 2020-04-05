#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
	char name[9];
	int size;
	int blckpnts[8];
	int used;
} INode;

typedef struct {
	char fbl[128];
	INode nodes[16];
} SuperBlock;

char *diskname;
int fdesc;
SuperBlock sb;

// Helper function to write the in-memory super block to disk
int writesb() {

	// Buffer for writing
	char buffer[1024];
	
	// Scanning position in buffer
	int pos = 0;

	// Insert free block list
	for(int i = 0; i < 128; i++) {
		buffer[pos++] = sb.fbl[i];
	}

	// Insert index nodes
	for(int i = 0; i < 16; i++) {

		// Insert name
		for(int j = 0; j < 8; j++) {
			buffer[pos++] = sb.nodes[i].name[j];
		}

		// Insert size
		for(int j = 0; j < 4; j++) {
			buffer[pos++] = (sb.nodes[i].size >> ((3 - j) << 3)) & 0xff;
		}

		// Insert block pointers
		for(int j = 0; j < 8; j++) {
			for(int k = 0; k < 4; k++) {
				buffer[pos++] = (sb.nodes[i].blckpnts[j] >> ((3 - k) << 3)) & 0xff;
			}
		}

		// Extract used boolean
		for(int j = 0; j < 4; j++) {
			buffer[pos++] = (sb.nodes[i].used >> ((3 - j) << 3)) & 0xff;
		}

	}

	// Write buffer to disk
	if(pwrite(fdesc, buffer, 1024, 0) == -1) {
		fprintf(stderr, "Writing %s failed: %i.\n", diskname, errno);
		return -1;
	}

	return 0;

}

// Create a new file in the file system
int fs_create(char name[], int size) {

	// Check input parameters
	if(strlen(name) > 8) {
		fprintf(stderr, "Cannot create file %s: file name too long.\n", name);
		return -1;
	}
	if(size > 8) {
		fprintf(stderr, "Cannot create file %s: requested size too large.\n", name);
		return -1;
	}
	
	// Search for a free node
	// Also check if file with given name already exists
	int freenode = -1;
	for(int i = 0; i < 16; i++) {
		if(sb.nodes[i].used == 0) {
			freenode = i;
		} else if(strcmp(sb.nodes[i].name, name) == 0) {
			fprintf(stderr, "Cannot create file %s: file named %s already exists.\n", name, sb.nodes[i].name);
			return -1;
		}
	}

	// Print failure message if there is no free node
	if(freenode == -1) {
		fprintf(stderr, "Cannot create file %s: maximum number of files reached.\n", name);
		return -1;
	}

	// Search for free blocks
	int freeblocks[size];
	int blockcount;
	for(int i = 0; i < 128 && blockcount < size; i++) {
		if(sb.fbl[i] == 0)
			freeblocks[blockcount++] = i;
	}

	// Print failure message if there are not enough free blocks
	if(blockcount < size) {
		fprintf(stderr, "Cannot create file %s: not enough free data blocks.\n", name);
		return -1;
	}

	// Allocate free node
	for(int i = 0; i < strlen(name); i++) {
		sb.nodes[freenode].name[i] = name[i];
	}
	sb.nodes[freenode].name[strlen(name)] = 0;
	sb.nodes[freenode].size = size;
	sb.nodes[freenode].used = 1;

	// Allocate free blocks
	for(int i = 0; i < size; i++) {
		sb.fbl[freeblocks[i]] = 1;
		sb.nodes[freenode].blckpnts[i] = freeblocks[i];
	}

	// Write super block changes to disk
	if(writesb() == -1)
		return -1;
	else
		printf("Creation complete for file %s.\n", name);

	return 0;

}

// Delete a file from the file system
int fs_delete(char name[]) {
	
	// Search for file with given name
	int nodeindex = -1;
	for(int i = 0; i < 16; i++) {
		if(sb.nodes[i].used != 0 && strcmp(sb.nodes[i].name, name) == 0) {
			nodeindex = i;
			break;
		}
	}
	if(nodeindex == -1) {
		fprintf(stderr, "Cannot delete file %s: no file of given name.\n", name);
		return -1;
	}

	// Free up blocks in free block list
	for(int i = 0; i < sb.nodes[nodeindex].size; i++) {
		sb.fbl[sb.nodes[nodeindex].blckpnts[i]] = 0;
	}

	// Free up node
	sb.nodes[nodeindex].used = 0;

	// Write super block changes to disk
	if(writesb() == -1)
		return -1;
	else
		printf("Deletion complete for file %s.\n", name);

	return 0;

}

// List files currently in the file system
int fs_ls() {
	
	// List of files from super block
	for(int i = 0; i < 16; i++) {
		if(sb.nodes[i].used)
			printf("%s %i\n", sb.nodes[i].name, sb.nodes[i].size);
	}

}

// Read data from a file in the file system
int fs_read(char name[], int blockNum, char buf[]) {
	// TODO: implementation
	return 0;
}

// Write data to a file in the file system
int fs_write(char name[], int blockNum, char buf[]) {
	// TODO: implementation
	return 0;
}

int main(int argc, char *argv[]) {

	// Check that we have an argument
	if (argc != 2 ) {
		fprintf(stderr, "usage: %s <diskFileName>\n", argv[0]);
		exit(1);
	}

	// Take filesystem name from args
	diskname = argv[1];
	
	// Open filesystem and store descriptor
	fdesc = open(argv[1], O_RDWR);
	if(fdesc == -1) {
		fprintf(stderr, "Opening %s failed: %i.\n", diskname, errno);
		exit(1);
	}

	// Parse super block from first 1KB
	printf("Reading super block of %s...", diskname);

	// Buffer for reading and writing
	char buffer[1024];

	// Read full super block into buffer
	if(pread(fdesc, buffer, 1024, 0) == -1) {
		fprintf(stderr, "Reading %s failed: %i.\n", diskname, errno);
		exit(1);
	}
	
	// Scanning position in buffer
	int pos = 0;

	// Extract free block list
	for(int i = 0; i < 128; i++) {
		sb.fbl[i] = buffer[pos++];
	}

	// Extract index nodes
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
				sb.nodes[i].blckpnts[j] <<= 8;
				sb.nodes[i].blckpnts[j] |= buffer[pos++];
			}
		}

		// Extract used boolean
		for(int j = 0; j < 4; j++) {
			sb.nodes[i].used <<= 8;
			sb.nodes[i].used |= buffer[pos++];
		}

	}

	printf(" Complete\n");

	// Actions on the filesystem can now be taken here
	fs_ls();

	// Close filesystem
	close(fdesc);

	return 0;

}