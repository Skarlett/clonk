#include <stdio.h>
#include <unistd.h>
#include "clonk.h"
#include "lexer.h"
#include "parser.h"

#define USER_IO_BUF 2048

int main() {
  ssize_t n = 1;
  char buffer[USER_IO_BUF];

  struct onk_lexer_input_t input;
  struct onk_lexer_output_t output;

  struct onk_parser_input_t parser_input;
  struct onk_parser_output_t parser_output;

  while(n > 0) {
      n = read(STDIN_FILENO, buffer, USER_IO_BUF);

      if (n > USER_IO_BUF)
      { n = USER_IO_BUF; }
      buffer[n] = 0;

      onk_tokenize(&input, &output);
      onk_parser_input_from_lexer_output(&output, &parser_input, false);
      onk_parse(&parser_input, &parser_output);

  }

  return 0;
}
