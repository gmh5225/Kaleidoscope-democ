#include <iostream>
#include <vector>
#include <memory>
#include <map>

//------------------------
// Lexer
//------------------------

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
    
    return last_char;
}

//------------------------
// Abstract Syntax Tree
//------------------------

namespace {
    // base class for all expression nodes
    class expr_AST
    {
    public:
        virtual ~expr_AST() = default;
    };

    // expression class for numeric literals like "1.0"
    class number_expr_AST : public expr_AST
    {
        double val;
    public:
        number_expr_AST(double val) : val(val) {}
    };

    // expression class for variables names like "a"
    class variable_expr_AST : public expr_AST
    {
        std::string name;
    public:
        variable_expr_AST(const std::string &name) : name(name) {}
    };

    // expression class for binary operator like "+"
    class binary_expr_AST : public expr_AST
    {
        char op;
        std::unique_ptr<expr_AST> lhs, rhs;

    public:
        binary_expr_AST(char op, std::unique_ptr<expr_AST> lhs,
            std::unique_ptr<expr_AST> rhs)
            : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    };

    // expression class for function calls
    class call_expr_AST : public expr_AST
    {
        std::string callee;
        std::vector<std::unique_ptr<expr_AST>> args;

    public:
        call_expr_AST(const std::string &callee,
                std::vector<std::unique_ptr<expr_AST>> args)
            : callee(callee), args(std::move(args)) {}
    };

    // prototype of a function
    class prototype_AST {
        std::string name;
        std::vector<std::string> args;

    public:
        prototype_AST(const std::string &name, std::vector<std::string> args)
            : name(name), args(std::move(args)) {}
        const std::string &get_name() const { return name; }
    };

    // function definition
    class function_AST {
        std::unique_ptr<prototype_AST> proto;
        std::unique_ptr<expr_AST> body;

    public:
        function_AST(std::unique_ptr<prototype_AST> proto,
            std::unique_ptr<expr_AST> body)
            : proto(std::move(proto)), body(std::move(body)) {}
    };

}

//------------------------
// Parser
//------------------------

// cur_tok is a token buffer
static int cur_tok;
// reads and updates cur_tok
static int get_next_token()
{
    return cur_tok = gettok();
}

// helper functions for error handling
std::unique_ptr<expr_AST> log_error(const char *str)
{
    fprintf(stderr, "Error: %s\n");
    return nullptr;
}

std::unique_ptr<prototype_AST> log_error_p(const char *str)
{
    log_error(str);
    return nullptr;
}

// This holds precedence for each binary operator defined
static std::map<char,int> binop_precedence;

// Get the precedence of the pending binary operator token
static int get_token_precedence()
{
    if(!isascii(cur_tok))
        return -1;
    
    int tok_prec = binop_precedence[cur_tok];
    if (binop_precedence[cur_tok] <= 0) return -1;
    return tok_prec;
}

// expression
//      ::= primary binorphs
static std::unique_ptr<expr_AST> parse_expression()
{
    auto lhs = parse_primary();
    if(!lhs)
        return nullptr;
    
    return parse_bin_op_rhs(0, std::move(lhs));
}

// numberexpr ::= number
static std::unique_ptr<expr_AST> parse_number_expr()
{
    auto result = std::make_unique<number_expr_AST>(num_val);
    get_next_token();
    return std::move(result);
}

// parseexpr ::= '(' expression ')'
static std::unique_ptr<expr_AST> parse_paren_expr() {
    get_next_token();
    auto v = parse_expression();
    if(!v)
        return nullptr;
    
    if(cur_tok != ')')
        return log_error("expected ')");
    get_next_token();
    return v;
}

// identifierexpr
//      ::= identifier
//      ::= identifier '(' expression* ')'
static std::unique_ptr<expr_AST> parse_identifier_expr()
{
    std::string id_name = identifier_str;

    get_next_token();

    if(cur_tok != '(')
        return std::make_unique<variable_expr_AST>(id_name);

    get_next_token();
    std::vector<std::unique_ptr<expr_AST>> args;
    if(cur_tok != ')') {
        while(true) {
            if(auto arg = parse_expression())
                args.push_back(std::move(arg));
            else
                return nullptr;

            if(cur_tok == ')')
                break;
            
            if(cur_tok != ',')
                return log_error("Expected ')' or ',' in argument list");
            get_next_token();
        }
    }
    get_next_token();
    return std::make_unique<call_expr_AST>(id_name, std::move(args));
}

// primary
//      ::= identifierexpr
//      ::= numberexpr
//      ::= parenexpr
static std::unique_ptr<expr_AST> parse_primary() {
    switch(cur_tok) {
        default:
            return log_error("unknown token when expecting an expression");
        case tok_identifier:
            return parse_identifier_expr();
        case tok_number:
            return parse_number_expr();
        case '(':
            return parse_paren_expr();
    }
}

// binoprhs
//      ::= ('+' primary)*
static std::unique_ptr<expr_AST> parse_bin_op_rhs(int expr_prec,
    std::unique_ptr<expr_AST> lhs)
{
    while(true) {
        int tok_prec = get_token_precedence();

        if(tok_prec < expr_prec)
            return lhs;

        int bin_op = cur_tok;
        get_next_token();

        auto rhs = parse_primary();
        if(!rhs)
            return nullptr;
        
        int next_prec = get_token_precedence();
        if(tok_prec < next_prec) {}

        lhs = std::make_unique<binary_expr_AST>(bin_op, std::move(lhs),
            std::move(rhs));
    }
}

// prototype
//      ::= id '(' id* ')'
static std::unique_ptr<prototype_AST> parse_prototype()
{
    if(cur_tok != tok_identifier)
        return log_error_p("expected function name in prototype");
    
    std::string fn_name = identifier_str;
    get_next_token();

    if(cur_tok != '(')
        return log_error_p("expected '(' in prototype");
    
    std::vector<std::string> arg_names;
    while(get_next_token() == tok_identifier)
        arg_names.push_back(identifier_str);
    if(cur_tok != ')')
        return log_error_p("Expected ')' in prototype");
    
    get_next_token();

    return std::make_unique<prototype_AST>(fn_name, std::move(arg_names));
}

std::unique_ptr<function_AST> parse_definition()
{
    get_next_token();
    auto proto = parse_prototype();
    if(!proto) return nullptr;

    if(auto E = parse_expression())
        return std::make_unique<function_AST>(std::move(proto), std::move(E));
    return nullptr;
}

// toplevelexpr ::= expression
static std::unique_ptr<function_AST> parse_top_level_expr()
{
    if(auto E = parse_expression()) {
        auto proto = std::make_unique<prototype_AST>("", std::vector<std::string>());
        return std::make_unique<function_AST>(std::move(proto), std::move(E));
    }
    return nullptr;
}

// external ::= 'extern' prototype
static std::unique_ptr<prototype_AST> parse_extern()
{
    get_next_token();
    return parse_prototype();
}

//------------------------
// Top-Level Parsing
//------------------------

static void handle_definition()
{
    if(parse_definition()) {
        fprintf(stderr, "parsed a function definition.\n");
    } else {
        get_next_token();
    }
}

static void handle_extern()
{
    if(parse_extern()) {
        fprintf(stderr, "parsed an extern.\n");
    } else {
        get_next_token();
    }
}

static void handle_top_level_expression()
{
    if(parse_top_level_expr()) {
        fprintf(stderr, "parsed a top-level expression.\n");
    } else {
        get_next_token();
    }
}

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
                break;
            case tok_extern:
                break;
            default:
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