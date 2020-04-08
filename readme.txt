Group 31: Lawrence Fritzler, Ryan Pinkney, Jeff Thomson
Github Link: https://github.com/ryanpinkney/Mini-Project-3

Part 1:

1) Design:
After reading the name of an input file via the console, calculates the page number and offset of each input entry using bitwise operations. Assumes that input lines will be in decimal form rather than hexidecimal or otherwise. Calculating page number is a straightforward bit shift to select the high-order bits. Calculating offset uses a bit-mask shifted right an appropriate amount of times, ANDed with the low-order bits to select only them.

2) Known Problems:
None

3) Build Instructions:
In a terminal, navigate to the project root directory and run the makefile with the command "make all". This will build both Part 1 and Part 2.

4) Run Instructions:
In a terminal, navigate to the Part-1 folder. Run the program with the command "./MemMan". The program will prompt the user for an input file to use.

Part 2:

1) Design:
The program is written within a single C file. It might have been prudent from a software development perspective to, early on, separate out some functionality into more files, but as-is it isn't too long and runs fine. The core disk functions are named fs_create, fs_delete, fs_ls, fs_read, and fs_write. There are also some helper functions, but much of the parsing done in the main method could be moved to a helper function.

The program takes an input file name as a program argument, and reads a disk file name from that input file. These two files will be kept open while the program is running. First the program reads the super-block into program memory from the disk file, then begins executing instructions as specified in the input file.

The core disk functions closely follow the specification in the handout lab3code.txt .

2) Known Problems:
There is an unusual problem with line 429 (printf("\nAction: C\n");). If the line is commented out, the program will encounter unpredictable segfaults while reading instructions from the input file, possibly due to I/O synchronization issues. Other similar print statements for other instructions do not seem to have this problem. The placement of a print statement there was initially an attempt at debugging, but since the progam runs fine so long as there is a print there, the design was modified so that there was a somewhat informative print there. It's something of a band-aid solution, but the code ultimately runs fine. This problem was replicated on more than one machine.

3) Build Instructions:
In a terminal, navigate to the project root directory and run the makefile with the command "make all". This will build both Part 1 and Part 2.

4) Run Instructions:
In a terminal, navigate to the Part-2 folder. Run the program with the command "./FileSys <input filename>", where <input filename> is a program argument for the input file to read from (not the disk filename, which will be read from the input file). Ensure that the desired input file is located in the Part-2 folder, as well as an appropriate disk file created by the handout program labs.create_fs.c .