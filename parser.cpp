#include "lexer.h"
#include "parser.h"
#include "util.h"

#include <map>

//------------------------
// Parser
//------------------------

// reads and updates cur_tok
static int get_next_token()
{
    return cur_tok = gettok();
}

// Get the precedence of the pending binary operator token
static int get_token_precedence()
{
    if(!isascii(cur_tok))
        return -1;
    
    int tok_prec = binop_precedence[cur_tok];
    if (binop_precedence[cur_tok] <= 0) return -1;
    return tok_prec;
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
        return log_error<prototype_AST>("expected function name in prototype");
    
    std::string fn_name = identifier_str;
    get_next_token();

    if(cur_tok != '(')
        return log_error<prototype_AST>("expected '(' in prototype");
    
    std::vector<std::string> arg_names;
    while(get_next_token() == tok_identifier)
        arg_names.push_back(identifier_str);
    if(cur_tok != ')')
        return log_error<prototype_AST>("Expected ')' in prototype");
    
    get_next_token();

    return std::make_unique<prototype_AST>(fn_name, std::move(arg_names));
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
                return log_error<expr_AST>("Expected ')' or ',' in argument list");
            get_next_token();
        }
    }
    get_next_token();
    return std::make_unique<call_expr_AST>(id_name, std::move(args));
}

// parseexpr ::= '(' expression ')'
static std::unique_ptr<expr_AST> parse_paren_expr() {
    get_next_token();
    auto v = parse_expression();
    if(!v)
        return nullptr;
    
    if(cur_tok != ')')
        return log_error<expr_AST>("expected ')");
    get_next_token();
    return v;
}

// primary
//      ::= identifierexpr
//      ::= numberexpr
//      ::= parenexpr
static std::unique_ptr<expr_AST> parse_primary() {
    switch(cur_tok) {
        default:
            return log_error<expr_AST>("unknown token when expecting an expression");
        case tok_identifier:
            return parse_identifier_expr();
        case tok_number:
            return parse_number_expr();
        case '(':
            return parse_paren_expr();
    }
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

static void handle_top_level_expression()
{
    if(parse_top_level_expr()) {
        fprintf(stderr, "parsed a top-level expression.\n");
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