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
                } binary;

                struct
                {
                        EXPRESSION *Operand;
                        TOKENTYPE Operator;
                } unary;

                struct
                {
                        SIZE Value;
                } integer_literal;

                struct
                {
                        char *Name;
                        EXPRESSION *Init;
                } declaration;
        } as;
        EXPRESSIONTYPE Type;
        EXPRESSION *Next;
};

struct AST
{
        SIZE ExprCount;
        EXPRESSION *RootExpr;
};

EXPRESSION *ParseStatements(AASState *State);
EXPRESSION *ParseExpression(AASState *State);
EXPRESSION *ParseTerm(AASState *State);
EXPRESSION *ParseFactor(AASState *State);
void DisplayExpression(EXPRESSION *Expr);
void DisplayExpressionTree(EXPRESSION *Expr, int);

#endif
