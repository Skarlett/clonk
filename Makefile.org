# Source, Executable, Includes, Library Defines

INCL  =

CUTEST = src/libtest/CuTest/CuStr.c 		\
		src/libtest/CuTest/CuTest.c 	\
		src/libtest/CuTest/suite.c

TESTLIB = src/libtest/build_mask.c		\
		src/libtest/tokens.c		\
		src/libtest/mask_assert.c	\
		src/libtest/textutil.c

TESTLIB_TEST = src/libtest/tests/test_assert.c		\
		src/libtest/tests/test_mask.c		\
		src/libtest/tests/CuTestTest.c

ONKSTD = src/onkstd/vec/vec.c 			\
		src/onkstd/queue/queue.c 	\
		src/onkstd/sort/sort_merge.c 	\
		src/onkstd/int.c

ONKSTD_TEST = src/onkstd/vec/tests.c 		\
		src/onkstd/queue/tests.c 	\
		src/onkstd/sort/tests.c

LEXER = src/parser/lexer/lexer.c 		\
		src/parser/lexer/helpers.c 	\
		src/parser/lexer/debug.c

LEXER_TEST = src/parser/lexer/tests/lexer_tests.c 	\
		src/parser/lexer/tests/harness.c

PARSER = src/parser/parser.c 	\
	src/parser/utils.c	\
	src/parser/predict.c 	\
	src/parser/error.c

PARSER_TEST = src/parser/tests/postfix.c \
	src/parser/tests/predict.c

SRC   = $(ONKSTD) 	\
	$(LEXER) 	\
	$(PARSER)


TESTS = $(CUTEST) 		\
	$(TESTLIB) 		\
	$(TESTLIB_TEST) 	\
	$(ONKSTD_TEST)		\
	$(LEXER_TEST)		\
	$(PARSER_TEST)

OBJ	= $(SRC:.c=.o)
LIBS 	=
EXE	= clonky
# Compiler, Linker Defines
CC	  = clang
CFLAGS  = -ansi -I "include/" -pedantic -fstack-protector-all -O3 $(LDFLAGS) -Wall

LIBPATH = -L.
LDFLAGS = -o $(EXE) $(LIBPATH) $(LIBS)
CFDEBUG = -ansi -I "include/" -pedantic -Wall -DDEBUG -Wcomment -ggdb -DINCLUDE_TESTS $(LDFLAGS)
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
	   $(CC) $(CFDEBUG) src/run_tests.c $(TESTS) $(SRC)

# Clean Up Objects, Exectuables, Dumps out of source directory
clean:
	   $(RM) $(OBJ) $(EXE) core a.out
