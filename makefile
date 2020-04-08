COMP = gcc

all: part-1 part-2

part-1: Part-1/MemMan.c
	$(COMP) -o Part-1/MemMan Part-1/MemMan.c

part-2: Part-2/FileSys.c
	$(COMP) -o Part-2/FileSys Part-2/FileSys.c