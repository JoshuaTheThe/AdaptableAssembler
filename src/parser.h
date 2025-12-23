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
        EXPR_TYPE_LITERAL_STR,
        EXPR_TYPE_LITERAL_REAL,
        EXPR_TYPE_DECLARATION,
        EXPR_TYPE_STRUCTURE,
        EXPR_TYPE_ACCESS,
        EXPR_TYPE_RETURN,
} EXPRESSIONTYPE;

typedef struct EXPRESSION EXPRESSION;

typedef EXPRESSION *PARAM;
typedef struct
{
        SIZE Count;
        PARAM *Params;
} PARAMS;

typedef struct
{
        // 0 - being value
        SIZE Depth;
        SIZE Dim[8]; /* Dimensions, 0 being value, e.g. 2x2 */
        SIZE DimCount;
} TYPEVARIANT;

typedef struct TYPE
{
        union
        {
                struct
                {
                        SIZE Bits;
                        BOOL Signed;
                } normal;
                struct
                {
                        char *StructureName;
                } structure;
        } as;
        TYPEVARIANT Variant;
        BOOL Constant;
        BOOL IsStructure;
} TYPE;

struct EXPRESSION
{
        union
        {
                struct
                {
                        EXPRESSION *Params;
                        EXPRESSION *Body;
                        TYPE ReturnType;
                        char *Name;
                } fun;

                struct
                {
                        char *Name;
                } variable;

                struct
                {
                        EXPRESSION *Lhs;
                        EXPRESSION *Rhs;
                } assignment;

                struct
                {
                        EXPRESSION *Callee;
                        EXPRESSION *Args;
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
                        EXPRESSION *Expr;
                        EXPRESSION *Index;
                } access;

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
                        double Value;
                } real_literal;

                struct
                {
                        const char *Data;
                        SIZE Value;
                } string_literal;

                struct
                {
                        TYPE Type;
                        EXPRESSION *Init;
                        char *Name;
                } declaration;

                struct
                {
                        EXPRESSION *Conditional, *Body, *ElseBody;
                } ifelse;

                struct
                {
                        EXPRESSION *Body;
                        char *Name;
                } structure;

                EXPRESSION *return_statement;
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
EXPRESSION *ParseAssignment(ArborState *State);
EXPRESSION *ParseIf(ArborState *State);
EXPRESSION *ParseStatement(ArborState *State);
EXPRESSION *ParsePrefix(ArborState *State);
EXPRESSION *ParseSuffix(ArborState *State, EXPRESSION *Expr);
EXPRESSION *ParseArguments(ArborState *State);
EXPRESSION *ParseDeclaration(ArborState *State);
TYPE ParseType(ArborState *State);
void DisplayExpressionTree(EXPRESSION *Expr, int);
EXPRESSION *ParseStruct(ArborState *State);

#endif
