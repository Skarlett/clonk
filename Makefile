# Source, Executable, Includes, Library Defines
INCL  = src/common.h \
		src/parser/lexer.h \
		src/parser/expr.h \
		src/parser/ast.h \
		src/productions/syn.h \
		src/common.h

SRC   = src/main.c \
		src/common.c \
		src/parser/lexer.c \
		src/parser/expr.c \
		src/parser/ast.c \
		src/productions/syn.c

OBJ	= $(SRC:.c=.o)
LIBS =
EXE	= moolicious
# Compiler, Linker Defines
CC	  = /usr/bin/clang
CFLAGS  = -ansi -pedantic -Wall -O2
LIBPATH = -L.
LDFLAGS = -o $(EXE) $(LIBPATH) $(LIBS)
CFDEBUG = -ansi -pedantic -DDEBUG $(LDFLAGS)
RM	  = /bin/rm -f

# compile under C99
ccflags-y := -std=c99


# Compile and Assemble C Source Files into Object Files
%.o: %.c
	   $(CC) -c $(CFLAGS) $*.c

# Link all Object Files with external Libraries into Binaries
$(EXE): $(OBJ)
	   $(CC) $(LDFLAGS) $(OBJ)

# Objects depend on these Libraries
$(OBJ): $(INCL)

# Create a gdb/dbx Capable Executable with DEBUG flags turned on
debug:
	   $(CC) $(CFDEBUG) $(SRC)

# Clean Up Objects, Exectuables, Dumps out of source directory
clean:
	   $(RM) $(OBJ) $(EXE) core a.out