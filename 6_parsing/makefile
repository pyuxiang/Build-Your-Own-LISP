# http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

# note relative directory path!
# directory paths to segregate types of files
# object directory for faster compilation given no changes
IDIR = include
SDIR = src
ODIR = obj

_OBJ = 6_parsing_lang.o 6_lang_set.o mpc.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ)) # accesses object directory

CC = gcc
CFLAGS = -std=c99 -Wall -I $(IDIR)

# -o $@ specifies object files to pass to left argument
# $< specifies first argument
# Object files: No need to specify dependencies due to -I flag
$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

parser: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

# del for cmd.exe, rm for bash shell
# Calling powershell from make.exe to execute rm command
clean:
	powershell rm $(ODIR)/*.o

clean_all:
	make clean
	del parser.exe
