#ifndef TOKEN_H
#define TOKEN_H

#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include"types.h"
#include"arch.h"

#define MAX_INST_LENGTH (1024)

TOKEN GetToken(ArborState *State);
void FreeToken(TOKEN *Token);
TOKEN ExpectToken(TOKEN Current, ArborState *State, TOKENTYPE Type);
TOKEN AcceptToken(TOKEN Current, ArborState *State, TOKENTYPE Type, BOOL *Success);
BOOL AcceptsToken(TOKEN Current, ArborState *State, TOKENTYPE Type);

#endif

