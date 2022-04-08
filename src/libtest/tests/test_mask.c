#include "clonk.h"
#include "lexer.h"
#include "libtest/CuTest.h"
#include "libtest/tokens.h"

void build_test_mold_kit(
    struct onk_test_mask_t *parser,
    struct onk_test_mask_t *lexer)
{
    const enum onk_lexicon_t static_answ[] = { ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_WORD_TOKEN, 0 };
    const enum onk_lexicon_t dyn_answ_s1[] = { ONK_MUL_TOKEN, ONK_DIV_TOKEN, 0 };
    const enum onk_lexicon_t dyn_answ_s2[] = { ONK_ADD_TOKEN, ONK_SUB_TOKEN, 0 };

    enum onk_lexicon_t working_space = ONK_WORD_TOKEN;

    // mold lexer output
    onk_desc_add_static_slot(lexer, &working_space, 1);
    onk_desc_add_dynamic_slot(lexer, (enum onk_lexicon_t *)dyn_answ_s2, 2);
    onk_desc_add_static_slot(lexer, &working_space, 1);
    onk_desc_add_dynamic_slot(lexer, (enum onk_lexicon_t *)dyn_answ_s1, 2);

    // Mold Parser output
    onk_desc_add_static_slot(parser,  (enum onk_lexicon_t *)static_answ, 3);
    onk_desc_add_dynamic_slot(parser, (enum onk_lexicon_t *)dyn_answ_s1, 2);
    onk_desc_add_dynamic_slot(parser, (enum onk_lexicon_t *)dyn_answ_s2, 2);
}


void example(CuTest *tc)
{
    struct onk_test_mask_t parser, lexer;
    struct onk_vec_t tokens;

    struct onk_lexer_input_t lexer_input;
    struct onk_desc_token_t lex_buf[12];
    struct onk_desc_token_t parse_buf[12];

    const char * fmt_segs[][2] =            \
        {{"+", "-"},{"*", "/"}};
        /*s1p1  s1p2  s2p1 s2p2*/

    const char * fmt = "word %s word %s word";

    char buf[256];

    onk_match_token_init(&lexer, (struct onk_desc_token_t *)lex_buf, 12);
    onk_match_token_init(&parser, (struct onk_desc_token_t *)parse_buf, 12);
    onk_vec_init(&tokens, 32, sizeof(struct onk_token_t));

    build_test_mold_kit(&parser, &lexer);

    lexer_input.tokens = tokens;

    for(int i=0; 2 > i; i++)
    {
        snprintf(buf, 256, fmt, fmt_segs[i][0], fmt_segs[i][1]);
        lexer_input.src_code = buf;

        OnkAssertParseStage(tc, &lexer, &parser, i);

        onk_vec_clear(&tokens);
    }
}
