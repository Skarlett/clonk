#include <stdio.h>

#include <emscripten/emscripten.h>


EMSCRIPTEN_KEEPALIVE void RunTestSuite(int argc, char ** argv) {
    dup2(file, stdout);

}
