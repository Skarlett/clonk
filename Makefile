# Source, Executable, Includes, Library Defines

INCL  =

SRC   = src/onkstd/vec/vec.c \
		src/onkstd/queue/queue.c \

		src/parser/lexer/lexer.c \
		src/parser/lexer/helpers.c \
		src/parser/lexer/debug.c \

		src/parser/parser.c \
		src/parser/utils.c \
		src/parser/predict.c \
		src/parser/handlers.c \
		src/parser/error.c


TESTLIB = src/libtest/CuTest.c 		\
		src/libtest/build_mask.c	\
		src/libtest/testutils.c



TESTS = src/libtest/tests/CuTestTest.c \
		src/libtests/tests/testutils_utils.c \

		src/onkstd/vec/tests.c \
		src/onkstd/queue/tests.c \

		src/parser/lexer/tests/lexer_tests.c \
		src/parser/lexer/tests/lexer_helpers.c

OBJ	= $(SRC:.c=.o)
LIBS =
EXE	= clonky
# Compiler, Linker Defines
CC	  = clang
CFLAGS  = -ansi -I "include/" -pedantic -fstack-protector-all -O3 $(LDFLAGS) -Wall

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
