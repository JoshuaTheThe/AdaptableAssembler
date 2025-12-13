#ifndef TOKEN_H
#define TOKEN_H

#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include"types.h"
#include"arch.h"

typedef enum
{
	TOKEN_NONE,
	TOKEN_NUMBER,
	TOKEN_IDENTIFIER,
	TOKEN_ASSEMBLY
} TOKENTYPE;

typedef struct
{
	STRING    Identifier;
	SIZE      Number;
	SIZE      Offset;
	TOKENTYPE Type;
	int       Padd;
} TOKEN;

TOKEN GetToken(AASState *State);

#endif

