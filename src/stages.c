#include "clonk.h"
#include "lexer.h"
#include "parser.h"

int token_stage(
    struct onk_parser_output_t *output,
    const char * text)
{
    struct onk_lexer_input_t input;
    struct onk_lexer_output_t token_output;
    struct onk_parser_input_t parser_input;

    if (onk_tokenize(&input, &token_output) == -1)
      return -1;

    onk_parser_input_from_lexer_output(&token_output, &parser_input, false);

    if (onk_parse(&parser_input, output) == -1)
      return -2;

    return 0;
}

int ast_stage() {}
