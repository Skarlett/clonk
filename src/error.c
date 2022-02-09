
#include <libgen.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "error.h"
#include "parser/expr/expr.h"

/*
* @param msg: NULL terminated str
*/
void _mk_error(
    struct Error *err_out,
    void *err_in,
    enum ErrorT in_type,
    enum Severity severity,
    const char *msg,
    const char *file,
) {
    char *dir_parent = 0, 
         *file_name = 0, 
         *dir_parent_name = 0;
    
    uint32_t len = 0;
    
    err_out->stage = ES_Unknown;
    err_out->severity = Fatal;
    err_out->type = ErrorUnknownT;
    err_out->msg = msg;
    err_out->file = file;
    err_out->type = in_type;

    assert(
        dir_parent != 0 
        || dir_parent_name != 0 
        || file_name != 0
        || msg != 0
    );

    if (err_out->type == ES_Parser)
      assert(memcpy(&err_out->data.parser, err_in, sizeof(struct ParseError)) != 0);
    else
    {
        printf("Unknown Error occured!");
        exit(1);
    }
}