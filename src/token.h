#ifndef TOKEN_H
#define TOKEN_H

#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include"types.h"
#include"arch.h"

#define MAX_INST_LENGTH (1024)

TOKEN GetToken(AASState *State);
void FreeToken(TOKEN *Token);
TOKEN ExpectToken(TOKEN Current, AASState *State, TOKENTYPE Type);
TOKEN AcceptToken(TOKEN Current, AASState *State, TOKENTYPE Type, BOOL *Success);
BOOL AcceptsToken(TOKEN Current, AASState *State, TOKENTYPE Type);

#endif

