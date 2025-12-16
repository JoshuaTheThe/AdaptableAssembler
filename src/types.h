#ifndef TYPES_H
#define TYPES_H

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<unistd.h>
#include<assert.h>

#define new(T)\
        Aalloc(sizeof (T))

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
	REAL   Version;
        SIZE   InstructionCount;
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
	STRING  Name;
	SIZE    CheckSum;
	SIZE    Offset;
	SEGMENT Segment;
	int     Padd;
} LABEL;

typedef enum
{
	TOKEN_NONE,
	TOKEN_NUMBER,
	TOKEN_IDENTIFIER,
	TOKEN_ASSEMBLY,
	TOKEN_LABEL,
	TOKEN_END,

        /* Assemble Time Expressions */
        TOKEN_EXPR_ADD,
        TOKEN_EXPR_SUB,
        TOKEN_EXPR_MUL,
        TOKEN_EXPR_DIV,
        TOKEN_EXPR_AND,
        TOKEN_EXPR_OR,
        TOKEN_EXPR_XOR,
        TOKEN_EXPR_LPAREN,
        TOKEN_EXPR_RPAREN,
} TOKENTYPE;

typedef struct TOKEN
{
        INSTRUCTION *Inst;
	STRING    Identifier;
	SIZE      Number;
	TOKENTYPE Type;
	int       Padd;
} TOKEN;

typedef struct
{
	/**
	 * Assume Worst case */
	LABEL *LabelsHead;
	LABEL *LabelsTail;
	SIZE   CheckSum;
	SIZE   LabelCount;
	FILE  *Assembly;
	FILE  *Output;
	ARCHITECTURE *Arch;
        TOKEN CurrentToken;
        void *Ast;
        void *CurrentExpr;
} AASState;

#ifndef NDEBUG
#define _assert(x) assert(x)
#else
#define _assert(x) if (!x) { printf("Assert Failed %s %s\n", __func__, __FILE__);  }
#endif

void *Aalloc(SIZE sz);

#endif

