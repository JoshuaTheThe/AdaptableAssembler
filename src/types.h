#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

#define new(T) \
        Aalloc(sizeof(T))

typedef size_t SIZE;
typedef char *STRING;
typedef _Bool BOOL;
typedef double REAL;

#define FALSE ((BOOL)0)
#define TRUE ((BOOL)1)

#define STATE_CHECKSUM ((SIZE)0xBADFBADF)
#define LABEL_CHECKSUM ((SIZE)0xBEEFBAD0)

typedef struct
{
        STRING Name;
        STRING Description;
        STRING Mnemonic;
        void (*Assemble)(void);
} INSTRUCTION;

typedef struct
{
        INSTRUCTION *Instructions;
        STRING Name;
        STRING Desc;
        REAL Version;
        SIZE InstructionCount;
} ARCHITECTURE;

typedef enum
{
        SEGMENT_TEXT,
        SEGMENT_DATA,
} SEGMENT;

typedef struct LABEL
{
        /**
         * Redundant Safety check.
         * I trust no one */
        struct LABEL *Next;
        struct LABEL *Prev;
        STRING Name;
        SIZE CheckSum;
        SIZE Offset;
        SEGMENT Segment;
        int Padd;
} LABEL;

typedef enum
{
        TOKEN_NONE,
        TOKEN_NUMBER,
        TOKEN_STRING,
        TOKEN_IDENTIFIER,
        TOKEN_LABEL,
        TOKEN_END,

        TOKEN_EXPR_ADD,
        TOKEN_EXPR_SUB,
        TOKEN_EXPR_MUL,
        TOKEN_EXPR_DIV,
        TOKEN_EXPR_MOD,
        TOKEN_EXPR_AND,
        TOKEN_EXPR_OR,
        TOKEN_EXPR_XOR,
        TOKEN_EXPR_LPAREN,
        TOKEN_EXPR_RPAREN,
        TOKEN_EXPR_LCPAREN,
        TOKEN_EXPR_RCPAREN,
        TOKEN_EXPR_EQ,
        TOKEN_EXPR_COMMA,
        TOKEN_EXPR_NOT,
        TOKEN_EXPR_COMPLEMENT,
        TOKEN_EXPR_LSPAREN,
        TOKEN_EXPR_RSPAREN,
        TOKEN_EXPR_LESS,
        TOKEN_EXPR_GREATER,
        TOKEN_EXPR_EQEQ,
        TOKEN_EXPR_CHAIN_AND,
        TOKEN_EXPR_CHAIN_OR,

        TOKEN_KEYWORDS,
        TOKEN_LET = TOKEN_KEYWORDS,
        TOKEN_FN,
        TOKEN_IF,
        TOKEN_ELSE,
        TOKEN_WHILE,
        TOKEN_FOR,
        TOKEN_INT,
        TOKEN_INT16,
        TOKEN_INT8,
        TOKEN_REAL,
        TOKEN_UNSIGNED,
        TOKEN_CONSTANT,
        TOKEN_STRUCT,
        TOKEN_RETURN,
        TOKEN_EXTERN,
} TOKENTYPE;

typedef struct TOKEN
{
        STRING Identifier;
        SIZE Number;
        REAL Real;
        TOKENTYPE Type;
        int Padd;
} TOKEN;

/* Parsing */

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
        EXPR_TYPE_EXTERN,
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
                        void *Vars;
                } fun;

                struct
                {
                        EXPRESSION *Params;
                        TYPE ReturnType;
                        char *Name;
                } ext;

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
        EXPRESSION *Parent;
        EXPRESSIONTYPE Type;
        TYPE CompileType;
        int padd;
};

struct AST
{
        SIZE ExprCount;
        EXPRESSION *RootExpr;
};

/* Code Generation */

struct VARIABLE;
typedef struct VARIABLE VARIABLE;

struct SCOPE;
struct STRUCTURE;
struct TSTRING;
typedef struct SCOPE SCOPE;
typedef struct STRUCTURE STRUCTURE;
typedef struct TSTRING TSTRING;

typedef struct VARIABLE
{
        TYPE Type;
        const char *Name;
        EXPRESSION *Expr;
        VARIABLE *Next, *Child, *Parent;
        long Address; /* EBP Offset or absolute */
        size_t SizeOf;
        BOOL IsOffset;
        BOOL IsParameter;
        BOOL IsFunction;
        BOOL IsAuto;
} VARIABLE;

typedef struct TSTRING
{
        const char *CStr;
        size_t Length;
        TSTRING *Next, *Child, *Parent;
} TSTRING;

typedef struct STRUCTURE
{
        const char *Name;
        EXPRESSION *Body;
        STRUCTURE *Next, *Child, *Parent;
} STRUCTURE;

typedef struct SCOPE
{
        VARIABLE *Vars;
        STRUCTURE *DataTypes;
        SCOPE *Next, *Child, *Parent;
        long BpOff;
} SCOPE;

typedef struct TRANSLATION
{
        SCOPE GlobalScope, *CurrentScope;
        TSTRING *StringTable;
        size_t StringCount;
} TRANSLATION;

/* Unifier */

typedef struct
{
        TRANSLATION TranslationUnit;
        LABEL *LabelsHead;
        LABEL *LabelsTail;
        SIZE CheckSum;
        SIZE LabelCount;
        FILE *Assembly;
        FILE *Output;
        ARCHITECTURE *Arch;
        TOKEN CurrentToken;
} ArborState;

#ifndef NDEBUG
#define _assert(x) assert(x)
#else
#define _assert(x)                                                   \
        if (!x)                                                      \
        {                                                            \
                printf("Assert Failed %s %s\n", __func__, __FILE__); \
        }
#endif

void *Aalloc(SIZE sz);

/**
 * T *x = ...
 * T *y = init_T(...)
 * ll_append(&x, y)
 */
#define ll_append(x, y)                                 \
        do                                              \
        {                                               \
                if (!(*(x)))                            \
                {                                       \
                        (*(x)) = y;                     \
                }                                       \
                else                                    \
                {                                       \
                        if ((*(x))->Next)               \
                        {                               \
                                y->Next = (*(x))->Next; \
                        }                               \
                        (*(x))->Next = y;               \
                }                                       \
        } while (0);

#define ll_append_child(x, y)                    \
        do                                       \
        {                                        \
                if ((*x)->Child)                 \
                {                                \
                        (y)->Next = (*x)->Child; \
                }                                \
                (*x)->Child = y;                 \
                (y)->Parent = (*x);              \
        } while (0)

#endif
