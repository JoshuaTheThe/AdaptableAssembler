#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "token.h"
#include "state.h"

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
EXPRESSION *ParseEqualityExpression(ArborState *State);

#endif
