#include "lexer.h"
#include "parser.h"

#include <iostream>

//------------------------
// Main Loop
//------------------------

static void main_loop()
{
    while(true) {
        fprintf(stderr, "ready> ");
        switch(cur_tok) {
            case tok_eof:
                return;
            case ';':
                get_next_token();
                break;
            case tok_def:
                handle_definition();
                break;
            case tok_extern:
                handle_extern();
                break;
            default:
                handle_top_level_expression();
                break;
        }
    }
}

// top ::= definition | external | expression | ';'
int main(int argc, char* argv[])
{
    binop_precedence['<'] = 10;
    binop_precedence['+'] = 20;
    binop_precedence['-'] = 30;
    binop_precedence['*'] = 40;

    fprintf(stderr, "ready> ");
    get_next_token();

    main_loop();

    return EXIT_SUCCESS;
}