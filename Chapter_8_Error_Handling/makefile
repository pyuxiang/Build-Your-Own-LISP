# http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

# note relative directory path!
# directory paths to segregate types of files
# object directory for faster compilation given no changes
IDIR = include
SDIR = src
ODIR = obj
LDIR = lib

CC = gcc
CFLAGS = -std=c99 -Wall -I $(IDIR) -I $(LDIR)

# -o $@ specifies object files to pass to left argument
# $< specifies first argument
# Object files: No need to specify dependencies due to -I flag
$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

# Library files
$(ODIR)/%.o: $(LDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)



# Normal build
_OBJ = language_parser.o language_set.o mpc.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ)) # accesses object directory

parser: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)



# Experimenting with bonus marks stuff
_SANDBOX_OBJ = language_parser_sandbox.o language_set.o mpc.o mpc_ast_misc.o
SANDBOX_OBJ = $(patsubst %, $(ODIR)/%, $(_SANDBOX_OBJ))

parser_sandbox: $(SANDBOX_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)



# del for cmd.exe, rm for bash shell
# Calling powershell from make.exe to execute rm command
clean:
	powershell rm $(ODIR)/*.o

clean_all:
	make clean
	del *.exe
