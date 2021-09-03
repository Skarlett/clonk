# Source, Executable, Includes, Library Defines
INCL  = 
SRC   = src/prelude.c \
		src/parser/lexer/lexer.c \
		src/parser/lexer/helpers.c \
		src/parser/expr/expr.c \
		src/parser/expr/debug.c \
		src/parser/ast.c \
		src/parser/synthetize.c

TESTS = tests/CuTest.c \
		tests/lexer_tests.c \
		tests/lexer_helpers.c \
		tests/common.c \
		tests/expr_tests.c

OBJ	= $(SRC:.c=.o)
LIBS =
EXE	= clonky
# Compiler, Linker Defines
CC	  = clang
CFLAGS  = -ansi -pedantic -Wall -O2 $(LDFLAGS)
LIBPATH = -L.
LDFLAGS = -o $(EXE) $(LIBPATH) $(LIBS)
CFDEBUG = -ansi -pedantic -Wall -DDEBUG -Wcomment -ggdb -DINCLUDE_TESTS $(LDFLAGS)
RM	  = /bin/rm -f

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
	   $(CC) $(CFDEBUG) src/main.c $(SRC)

# Create a gdb/dbx Capable Executable with DEBUG flags turned on
test:
	   $(CC) $(CFDEBUG) tests/run_tests.c $(TESTS) $(SRC)

# Clean Up Objects, Exectuables, Dumps out of source directory
clean:
	   $(RM) $(OBJ) $(EXE) core a.out
