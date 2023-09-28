#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>

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

// cur_tok is a token buffer
static int cur_tok;

// This holds precedence for each binary operator defined
static std::map<char,int> binop_precedence;

//------------------------
// Top-Level Parsing functions
//------------------------
static void handle_definition();

static void handle_top_level_expression();

static void handle_extern();


static int get_next_token();