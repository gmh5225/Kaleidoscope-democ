// starting with hello world!

#include <iostream>

// Lexer
// The lexer returns [0-255] ASCII value if it is not a known character
enum token {
    tok_eof = -1,

    // commands
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5,
};

static int gettok() {
    static int last_char = ' ';

    // skip whitespace
    while(isspace(last_char))
        last_char = getchar();

    if(last_char == '#') { // comment lines starting with # will be skipped
        do
            last_char = getchar();
        while (last_char != EOF && last_char != '\n' && last_char != '\r');
        if(last_char != EOF)
            return gettok();
    }

    if(isdigit(last_char) || last_char == '.') { // number: [0-9.]+
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
    
    int this_char = last_char;
    return this_char;
}

static std::string identifier_str;
static double num_val;

int main(int argc, char* argv[])
{
    std::cout << "hello world!";
}