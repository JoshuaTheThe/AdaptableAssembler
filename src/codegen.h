#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
#include "label.h"

VARIABLE *NewVariable(ArborState *State, TYPE Type, char *Name, BOOL IsOffset, int Address);
void AppendVariable(VARIABLE **Variables, VARIABLE *Variable);
TYPE GenerateForExpr(ArborState *State, EXPRESSION *Expr);
void EmitStart(ArborState *State);
void FindFunctions(ArborState *State, EXPRESSION *Expr);

#endif
