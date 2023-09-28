#pragma once

#include <string>

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

static std::string identifier_str;
static double num_val;

static int gettok();