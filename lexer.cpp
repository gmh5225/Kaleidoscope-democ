#include "lexer.h"

#include <string>

//------------------------
// Lexer
//------------------------

// matches tokens and returns
static int gettok() {
    static int last_char = ' ';

    while(isspace(last_char))
        last_char = getchar();

    if(last_char == '#') { // comment lines starting with # will be skipped
        do
            last_char = getchar();
        while (last_char != EOF && last_char != '\n' && last_char != '\r');
        if(last_char != EOF)
            return gettok();
    }

    if(isdigit(last_char)) { // number: [0-9.]+
        std::string num_str;
        do {
            num_str += last_char;
            last_char = getchar();
        } while(isdigit(last_char) || last_char == '.');

        num_val = strtod(num_str.c_str(), 0);
        return tok_number;
    }
    
    if(isalpha(last_char)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        identifier_str = last_char;
        while(isalnum(last_char = getchar()))
            identifier_str += last_char;
        if(identifier_str == "def")
            return tok_def;
        if(identifier_str == "extern")
            return tok_extern;
        return tok_identifier;
    }
    
    // if none of the above match
    if(last_char == EOF)
        return tok_eof;
    
    return getchar();
}