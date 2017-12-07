# http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

# note relative directory path!
# directory paths to segregate types of files
# object directory for faster compilation given no changes
IDIR = include
SDIR = src
ODIR = obj
LDIR = lib

CC = gcc
IFLAGS = -I $(IDIR) -I $(IDIR)/polish_lang_set \
	-I $(SDIR) -I $(SDIR)/polish_lang_set -I $(LDIR)
CFLAGS = -std=c99 -Wall $(IFLAGS)

# -o $@ specifies object files to pass to left argument
# $< specifies first argument
# Object files: No need to specify dependencies due to -I flag
$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

# Library files
$(ODIR)/%.o: $(LDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)



# Normal build
_OBJ = main.o readline_history.o mpc.o lang_parser_set.o \
	polish_lang_set/lang_set.o \
	polish_lang_set/lval_lenv.o \
	polish_lang_set/builtin.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ)) # accesses object directory

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	make main
	make clear

# del for cmd.exe, rm for bash shell
# Calling powershell from make.exe to execute rm command
clear:
	powershell rm $(ODIR)/*.o
	powershell rm $(ODIR)/polish_lang_set/*.o

clear_all:
	make clear
	del *.exe

# To consult this later on subdirectories:
# https://stackoverflow.com/questions/231229/how-to-generate-a-makefile-with-source-in-sub-directories-using-just-one-makefil
# Also, to add rule to create subdirectories if they do not exist
