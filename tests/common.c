#include "../src/parser/lexer/lexer.h"
#include "../src/prelude.h"

int __check_tokens(struct Token tokens[], enum Lexicon lexicon[], usize len){
    for (int i=0; len > i; i++) {
        if (tokens[i].type != lexicon[i]) {
            return -1;
        }
    }
    return 0;
}


int __check_tokens_by_ref(struct Token *tokens[], enum Lexicon lexicon[], usize len){
    for (int i=0; len > i; i++) {
        if (tokens[i]->type != lexicon[i]) {
            return -1;
        }
    }
    return 0;
}

