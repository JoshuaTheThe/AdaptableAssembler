#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "token.h"
#include "state.h"

typedef enum
{
        EXPR_TYPE_NONE,
        EXPR_TYPE_VAR,
        EXPR_TYPE_FUNCTION,
        EXPR_TYPE_ASSIGNMENT,
        EXPR_TYPE_CALL,
        EXPR_TYPE_BINARY_OP,
        EXPR_TYPE_IFELSE,
        EXPR_TYPE_UNARY_OP,
        EXPR_TYPE_LITERAL_NUM,
        EXPR_TYPE_DECLARATION,
} EXPRESSIONTYPE;

typedef struct EXPRESSION EXPRESSION;

typedef char *PARAM;
typedef struct
{
        SIZE Count;
        PARAM *Params;
} PARAMS;

struct EXPRESSION
{
        union
        {
                struct
                {
                        PARAMS Params;
                        EXPRESSION *Body;
                        char *Name;
                } fun;

                struct
                {
                        char *Name;
                } var;

                struct
                {
                        EXPRESSION *Lhs;
                        EXPRESSION *Rhs;
                } assignment;

                struct
                {
                        EXPRESSION *Callee;
                        EXPRESSION **Args;
                        SIZE ArgCount;
                } call;

                struct
                {
                        EXPRESSION *Lhs;
                        EXPRESSION *Rhs;
                        TOKENTYPE Operator;
                        int padd;
                } binary;

                struct
                {
                        EXPRESSION *Operand;
                        TOKENTYPE Operator;
                        int padd;
                } unary;

                struct
                {
                        SIZE Value;
                } integer_literal;

                struct
                {
                        EXPRESSION *Init;
                        char *Name;
                } declaration;

                struct
                {
                        EXPRESSION *Conditional, *Body, *ElseBody;
                } ifelse;
        } as;
        EXPRESSION *Next;
        EXPRESSIONTYPE Type;
        int padd;
};

struct AST
{
        SIZE ExprCount;
        EXPRESSION *RootExpr;
};

EXPRESSION *ParseFunction(ArborState *State);
EXPRESSION *ParseStatements(ArborState *State);
EXPRESSION *ParseExpression(ArborState *State);
EXPRESSION *ParseTerm(ArborState *State);
EXPRESSION *ParseFactor(ArborState *State);
void DisplayExpression(EXPRESSION *Expr);
void DisplayExpressionTree(EXPRESSION *Expr, int);
EXPRESSION *ParseAssignment(ArborState *State);
EXPRESSION *ParseIf(ArborState *State);
EXPRESSION *ParseStatement(ArborState *State);

#endif
