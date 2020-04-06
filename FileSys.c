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
char *input;
int diskfd;
int inputfd;
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
	if(pwrite(diskfd, buffer, 1024, 0) == -1) {
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

	// It might be prudent to zero out the blocks either here or in deletion, as
	// currently there may be leftover data that would be accessible if fs_read()
	// was called before fs_write().

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
	//printf("Files:\n");
	for(int i = 0; i < 16; i++) {
		if(sb.nodes[i].used)
			printf("%s %i\n", sb.nodes[i].name, sb.nodes[i].size);
	}

	return 0;

}

// Read data from a file in the file system
int fs_read(char name[], int blocknum, char buf[]) {
	
	// Search for file with given name
	int nodeindex = -1;
	for(int i = 0; i < 16; i++) {
		if(sb.nodes[i].used != 0 && strcmp(sb.nodes[i].name, name) == 0) {
			nodeindex = i;
			break;
		}
	}
	if(nodeindex == -1) {
		fprintf(stderr, "Cannot read from file %s: no file of given name.\n", name);
		return -1;
	}

	// Calculate offset
	int offset = 1024 * sb.nodes[nodeindex].blckpnts[blocknum];

	// Read into buffer from block
	if(pread(diskfd, buf, 1024, offset) == -1) {
		fprintf(stderr, "Failed to read from file %s: %i\n", name, errno);
		return -1;
	} else {
		printf("Finished reading from file %s.\n", name);
	}

	return 0;
}

// Write data to a file in the file system
int fs_write(char name[], int blocknum, char buf[]) {

	// Search for file with given name
	int nodeindex = -1;
	for(int i = 0; i < 16; i++) {
		if(sb.nodes[i].used != 0 && strcmp(sb.nodes[i].name, name) == 0) {
			nodeindex = i;
			break;
		}
	}
	if(nodeindex == -1) {
		fprintf(stderr, "Cannot write to file %s: no file of given name.\n", name);
		return -1;
	}

	// Calculate offset
	int offset = sb.nodes[nodeindex].blckpnts[blocknum] << 10;

	// Write from buffer into file
	if(pwrite(diskfd, buf, 1024, offset) == -1) {
		fprintf(stderr, "Failed to write to file %s: %i\n", name, errno);
		return -1;
	} else {
		printf("Finished writing to file %s.\n", name);
	}
	
	return 0;

}

// Read the line of the file given by fd, starting from offset, into a buffer of size 32.
// Returns offset of first character from next line.
int readline(int fd, char buffer[], int offset) {

	// Read bytes from file
	int bytesread = pread(fd, buffer, 32, offset);

	// Check if there was a failure or end of file
	if(bytesread < 0) {
		fprintf(stderr, "Could not read from file.\n");
		return -1;
	}
	if(bytesread == 0) {
		return -1;
	}

	// Search for the end of line within buffer
	int lineindex = 0;
	while(lineindex < bytesread) {
		if(buffer[lineindex] == '\n' || buffer[lineindex] == '\r') {
			buffer[lineindex++] = '\0';
			break;
		}
		lineindex++;
	}

	// If the line goes on to end of file, add null terminator to buffer
	if(lineindex == bytesread) {
		buffer[lineindex + 1] = '\0';
		return offset + lineindex + 1;
	}

	// Calculate the starting index of the next line
	int nextindex = lineindex;
	while(buffer[nextindex] == '\n' || buffer[nextindex] == '\r') {
		nextindex++;
	}

	return offset + nextindex;

}

int main(int argc, char *argv[]) {

	// Check that we have an argument
	if (argc != 2 ) {
		fprintf(stderr, "usage: %s <input file name>\n", argv[0]);
		exit(1);
	}

	// Take input file name from args
	input = argv[1];
	
	// Open input file and store descriptor
	inputfd = open(input, O_RDWR | O_DSYNC);
	if(inputfd == -1) {
		fprintf(stderr, "Opening %s failed: %i.\n", diskname, errno);
		exit(1);
	}

	// Setup variables to read input file line by line
	char linebuffer[32];
	int nextline = 0;

	// Read disk name from input file
	nextline = readline(inputfd, linebuffer, 0);
	if(nextline == -1) {
		fprintf(stderr, "Could not read disk name from input file.");
		exit(1);
	}
	char diskname[strlen(linebuffer) + 1];
	strcpy(diskname, linebuffer);

	// Parse super block from first 1KB
	printf("Reading super block of %s...", diskname);

	// Open filesystem and store descriptor
	diskfd = open(diskname, O_RDWR | O_DSYNC);
	if(diskfd == -1) {
		fprintf(stderr, "Opening %s failed: %i.\n", diskname, errno);
		exit(1);
	}

	// Buffer for reading and writing
	char buffer[1024];

	// Read full super block into buffer
	if(pread(diskfd, buffer, 1024, 0) == -1) {
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

	// Start reading and executing instructions from input file
	nextline = readline(inputfd, linebuffer, nextline);
	while(nextline != -1) {

		//sleep(1);

		// Perform instruction based on first character of line
		char action = linebuffer[0];
		if(action == 'L') {

			fs_ls();

		} else if(action == 'C') {

			// Read name from line buffer
			int namelen = strlen(linebuffer) - 4;
			char name[namelen + 1];
			name[namelen] = '\0';
			//printf("test\n"); // If this line is omitted there seem to be I/O synchronization issues.
			for(int i = 0; i < namelen; i++) {
				name[i] = linebuffer[i + 2];
			}

			// Read size from line buffer
			int size = linebuffer[strlen(linebuffer) - 1] - '0';

			fs_create(name, size);

		} else if(action == 'D') {

			// Read name from line buffer
			int namelen = strlen(linebuffer) - 2;
			char name[namelen + 1];
			name[namelen] = '\0';
			for(int i = 0; i < namelen; i++) {
				name[i] = linebuffer[i + 2];
			}

			fs_delete(name);

		} else if(action == 'R') {

			// Read name from line buffer
			int namelen = strlen(linebuffer) - 4;
			char name[namelen + 1];
			name[namelen] = '\0';
			for(int i = 0; i < namelen; i++) {
				name[i] = linebuffer[i + 2];
			}

			// Get position from line buffer
			int readpos = linebuffer[strlen(linebuffer) - 1] - '0';

			// Set up dummy buffer
			char readbuffer[1024];

			fs_read(name, readpos, readbuffer);

		} else if(action == 'W') {

			// Read name from line buffer
			int namelen = strlen(linebuffer) - 4;
			char name[namelen + 1];
			name[namelen] = '\0';
			for(int i = 0; i < namelen; i++) {
				name[i] = linebuffer[i + 2];
			}

			// Get position from line buffer
			int writepos = linebuffer[strlen(linebuffer) - 1] - '0';

			// Set up dummy buffer
			char writebuffer[] = "I learned a lot from COSC 315!";

			fs_write(name, writepos, writebuffer);

		} else {
			fprintf(stderr, "Unrecognized instruction: %c", action);
			exit(1);
		}

		// Read new line from file
		nextline = readline(inputfd, linebuffer, nextline);

	}

	// Close filesystem
	close(inputfd);
	close(diskfd);

	return 0;

}