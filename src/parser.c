#include "parser.h"

_Noreturn static void TODO(const char *x)
{
        printf("TODO: %s\n", x);
        exit(1);
}

static SIZE Count(EXPRESSION *Expr)
{
        SIZE k = 0;
        while (Expr)
                (void)k++, Expr = Expr->Next;
        return k;
}

static EXPRESSION *Literal(SIZE Value)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_LITERAL_NUM;
        Expr->as.integer_literal.Value = Value;
        return (Expr);
}

static EXPRESSION *RealLiteral(REAL Value)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_LITERAL_REAL;
        Expr->as.real_literal.Value = Value;
        return (Expr);
}

static EXPRESSION *StringLiteral(const char *Data)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_LITERAL_STR;
        Expr->as.string_literal.Data = Data;
        return (Expr);
}

static EXPRESSION *Variable(char *Name)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_VAR;
        Expr->as.variable.Name = Name;
        return (Expr);
}

static EXPRESSION *Function(char *Name, EXPRESSION *Body, EXPRESSION *Params, TYPE ReturnType)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_FUNCTION;
        Expr->as.fun.Name = Name;
        Expr->as.fun.Body = Body;
        Expr->as.fun.Params = Params;
        Expr->as.fun.ReturnType = ReturnType;
        return (Expr);
}

static EXPRESSION *Assignment(EXPRESSION *Lhs, EXPRESSION *Rhs)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_ASSIGNMENT;
        Expr->as.assignment.Lhs = Lhs;
        Expr->as.assignment.Rhs = Rhs;
        return (Expr);
}

static EXPRESSION *Binary(EXPRESSION *Lhs, EXPRESSION *Rhs, TOKENTYPE Op)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_BINARY_OP;
        Expr->as.binary.Lhs = Lhs;
        Expr->as.binary.Rhs = Rhs;
        Expr->as.binary.Operator = Op;
        return (Expr);
}

static EXPRESSION *If(EXPRESSION *Body, EXPRESSION *ElseBody, EXPRESSION *Conditional)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_IFELSE;
        Expr->as.ifelse.Conditional = Conditional;
        Expr->as.ifelse.Body = Body;
        Expr->as.ifelse.ElseBody = ElseBody;
        return (Expr);
}

static EXPRESSION *Unary(EXPRESSION *UnaryExpr, TOKENTYPE Op)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_UNARY_OP;
        Expr->as.unary.Operand = UnaryExpr;
        Expr->as.unary.Operator = Op;
        return (Expr);
}

static EXPRESSION *Declaration(char *Name, EXPRESSION *Init, TYPE Type)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_DECLARATION;
        Expr->as.declaration.Init = Init;
        Expr->as.declaration.Name = Name;
        Expr->as.declaration.Type = Type;
        return (Expr);
}

static EXPRESSION *Structure(char *Name, EXPRESSION *Body)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_STRUCTURE;
        Expr->as.structure.Body = Body;
        Expr->as.structure.Name = Name;
        return (Expr);
}

static EXPRESSION *Call(EXPRESSION *Callee, EXPRESSION *Args)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_CALL;
        Expr->as.call.Callee = Callee;
        Expr->as.call.Args = Args;
        Expr->as.call.ArgCount = Count(Args);
        return (Expr);
}

static EXPRESSION *Access(EXPRESSION *Expression, EXPRESSION *Index)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_ACCESS;
        Expr->as.access.Expr = Expression;
        Expr->as.access.Index = Index;
        return (Expr);
}

static EXPRESSION *Return(EXPRESSION *Expression)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_RETURN;
        Expr->as.return_statement = Expression;
        return (Expr);
}

static void AppendExpr(EXPRESSION **restrict Dest, EXPRESSION *restrict Source)
{
        EXPRESSION *End = *Dest;
        if (*Dest == NULL)
        {
                *Dest = Source;
                return;
        }

        while (End->Next)
        {
                End = End->Next;
        }
        End->Next = Source;
}

EXPRESSION *ParseFactor(ArborState *State)
{
        EXPRESSION *Expr = NULL;

        if (AcceptsToken(State->CurrentToken, State, TOKEN_NUMBER))
        {
                Expr = Literal(State->CurrentToken.Number);
                State->CurrentToken = GetToken(State);
        }
        else if (AcceptsToken(State->CurrentToken, State, TOKEN_STRING))
        {
                Expr = StringLiteral(State->CurrentToken.Identifier);
                State->CurrentToken = GetToken(State);
        }
        else if (AcceptsToken(State->CurrentToken, State, TOKEN_REAL))
        {
                Expr = RealLiteral(State->CurrentToken.Real);
                State->CurrentToken = GetToken(State);
        }
        else if (AcceptsToken(State->CurrentToken, State, TOKEN_EXPR_LPAREN))
        {
                State->CurrentToken = GetToken(State);
                Expr = ParseExpression(State);
                State->CurrentToken = ExpectToken(State->CurrentToken, State, TOKEN_EXPR_RPAREN);
        }
        else if (AcceptsToken(State->CurrentToken, State, TOKEN_IDENTIFIER))
        {
                char *Name = strdup(State->CurrentToken.Identifier);
                Expr = Variable(Name);
                State->CurrentToken = GetToken(State);
        }

        return Expr;
}

EXPRESSION *ParseArguments(ArborState *State)
{
        EXPRESSION *Expressions = NULL, *Expr = NULL;

        if (State->CurrentToken.Type == TOKEN_EXPR_RPAREN)
                return NULL;

        Expr = ParseExpression(State);
        AppendExpr(&Expressions, Expr);

        while (State->CurrentToken.Type == TOKEN_EXPR_COMMA)
        {
                State->CurrentToken = GetToken(State);
                Expr = ParseExpression(State);
                AppendExpr(&Expressions, Expr);
        }

        return Expressions;
}

EXPRESSION *ParseSuffix(ArborState *State, EXPRESSION *Expr)
{
        EXPRESSION *Initial = Expr, *Arguments;
        if (State->CurrentToken.Type == TOKEN_EXPR_LPAREN)
        {
                State->CurrentToken = GetToken(State);
                Arguments = ParseArguments(State);
                State->CurrentToken = ExpectToken(State->CurrentToken, State, TOKEN_EXPR_RPAREN);
                Expr = Call(Initial, Arguments);
                return ParseSuffix(State, Expr);
        }
        else if (State->CurrentToken.Type == TOKEN_EXPR_LSPAREN)
        {
                State->CurrentToken = GetToken(State);
                Arguments = ParseArguments(State);
                State->CurrentToken = ExpectToken(State->CurrentToken, State, TOKEN_EXPR_RSPAREN);
                Expr = Access(Initial, Arguments);
                return ParseSuffix(State, Expr);
        }

        return Expr;
}

EXPRESSION *ParsePrefix(ArborState *State)
{
        EXPRESSION *Expr = NULL;
        if (State->CurrentToken.Type == TOKEN_EXPR_SUB)
        {
                State->CurrentToken = GetToken(State);
                Expr = Unary(ParseFactor(State), TOKEN_EXPR_SUB);
        }
        else if (State->CurrentToken.Type == TOKEN_EXPR_NOT)
        {
                State->CurrentToken = GetToken(State);
                Expr = Unary(ParseFactor(State), TOKEN_EXPR_NOT);
        }
        else if (State->CurrentToken.Type == TOKEN_EXPR_COMPLEMENT)
        {
                State->CurrentToken = GetToken(State);
                Expr = Unary(ParseFactor(State), TOKEN_EXPR_COMPLEMENT);
        }
        else if (State->CurrentToken.Type == TOKEN_EXPR_MUL)
        {
                State->CurrentToken = GetToken(State);
                Expr = Unary(ParseFactor(State), TOKEN_EXPR_MUL);
        }
        else
        {
                Expr = ParseFactor(State);
        }

        Expr = ParseSuffix(State, Expr);
        return Expr;
}

EXPRESSION *ParseTerm(ArborState *State)
{
        EXPRESSION *Left = ParsePrefix(State), *Right = NULL;
        while (State->CurrentToken.Type == TOKEN_EXPR_MUL ||
               State->CurrentToken.Type == TOKEN_EXPR_DIV)
        {
                TOKENTYPE Op = State->CurrentToken.Type;
                State->CurrentToken = GetToken(State);

                Right = ParsePrefix(State);
                Left = Binary(Left, Right, Op);
        }

        return Left;
}

EXPRESSION *ParseExpression(ArborState *State)
{
        EXPRESSION *Left = ParseTerm(State), *Right = NULL;

        while (State->CurrentToken.Type == TOKEN_EXPR_ADD ||
               State->CurrentToken.Type == TOKEN_EXPR_SUB)
        {
                TOKENTYPE Op = State->CurrentToken.Type;
                State->CurrentToken = GetToken(State);
                Right = ParseTerm(State);
                Left = Binary(Left, Right, Op);
        }

        return Left;
}

EXPRESSION *ParseAssignment(ArborState *State)
{
        EXPRESSION *Left = ParseExpression(State);
        EXPRESSION *Right = NULL;
        if (State->CurrentToken.Type == TOKEN_EXPR_EQ)
        {
                State->CurrentToken = GetToken(State);
                Right = ParseExpression(State);
                return Assignment(Left, Right);
        }

        return Left;
}

EXPRESSION *ParseFunction(ArborState *State)
{
        char *Name = strdup(State->CurrentToken.Identifier);
        EXPRESSION *Params = {0};
        EXPRESSION *Body = NULL;
        State->CurrentToken = GetToken(State);
        State->CurrentToken = ExpectToken(State->CurrentToken, State, TOKEN_EXPR_LPAREN);
        while (State->CurrentToken.Type != TOKEN_EXPR_RPAREN)
        {
                if (State->CurrentToken.Type == TOKEN_IDENTIFIER)
                {
                        char *Name1 = strdup(State->CurrentToken.Identifier);
                        State->CurrentToken = GetToken(State);
                        TYPE Type = ParseType(State);
                        AppendExpr(&Params, Declaration(Name1, NULL, Type));
                        if (State->CurrentToken.Type != TOKEN_EXPR_RPAREN)
                                State->CurrentToken = ExpectToken(State->CurrentToken, State, TOKEN_EXPR_COMMA);
                }
        }
        State->CurrentToken = ExpectToken(State->CurrentToken, State, TOKEN_EXPR_RPAREN);
        TYPE Type = ParseType(State);
        Body = ParseStatement(State);
        return Function(Name, Body, Params, Type);
}

EXPRESSION *ParseIf(ArborState *State)
{
        EXPRESSION *Body, *Expr, *ElseBody = NULL;
        Expr = ParseExpression(State);
        Body = ParseStatement(State);

        if (State->CurrentToken.Type == TOKEN_ELSE)
        {
                State->CurrentToken = GetToken(State);
                ElseBody = ParseStatement(State);
        }

        return If(Body, ElseBody, Expr);
}

TYPE ParseType(ArborState *State)
{
        TYPE Type = {0};
        while (State->CurrentToken.Type == TOKEN_EXPR_MUL)
        {
                State->CurrentToken = GetToken(State);
                Type.Variant.Depth += 1;
        }
        while (State->CurrentToken.Type == TOKEN_EXPR_LSPAREN)
        {
                State->CurrentToken = GetToken(State);
                if (State->CurrentToken.Type != TOKEN_NUMBER)
                {
                        TODO("dimension length must be a constant");
                }
                Type.Variant.Dim[Type.Variant.DimCount++] = State->CurrentToken.Number;
                State->CurrentToken = GetToken(State);
                State->CurrentToken = ExpectToken(State->CurrentToken, State, TOKEN_EXPR_RSPAREN);
        }
        if (State->CurrentToken.Type == TOKEN_CONSTANT)
        {
                Type.Constant = TRUE;
                State->CurrentToken = GetToken(State);
        }

        if (State->CurrentToken.Type == TOKEN_UNSIGNED)
        {
                Type.as.normal.Signed = FALSE;
                State->CurrentToken = GetToken(State);
        }
        else
        {
                Type.as.normal.Signed = TRUE;
        }

        if (State->CurrentToken.Type == TOKEN_INT)
        {
                Type.IsStructure = FALSE;
                Type.as.normal.Bits = 32;
                State->CurrentToken = GetToken(State);
        }
        else if (State->CurrentToken.Type == TOKEN_INT16)
        {
                Type.IsStructure = FALSE;
                Type.as.normal.Bits = 16;
                State->CurrentToken = GetToken(State);
        }
        else if (State->CurrentToken.Type == TOKEN_INT8)
        {
                Type.IsStructure = FALSE;
                Type.as.normal.Bits = 8;
                State->CurrentToken = GetToken(State);
        }
        else if (State->CurrentToken.Type == TOKEN_IDENTIFIER)
        {
                Type.IsStructure = TRUE;
                Type.as.structure.StructureName = strdup(State->CurrentToken.Identifier);
                State->CurrentToken = GetToken(State);
        }
        else
        {
                TODO("variable must have a type");
        }

        return Type;
}

EXPRESSION *ParseDeclaration(ArborState *State)
{
        char *Name = NULL;
        EXPRESSION *Init = NULL;
        TYPE Type = {0};
        if (State->CurrentToken.Type != TOKEN_IDENTIFIER)
        {
                TODO("Errors -- Declarations.Ident");
                return NULL;
        }

        Name = strdup(State->CurrentToken.Identifier);
        State->CurrentToken = GetToken(State);
        Type = ParseType(State);

        if (AcceptsToken(State->CurrentToken, State, TOKEN_EXPR_EQ))
        {
                State->CurrentToken = GetToken(State);
                Init = ParseExpression(State);
        }

        State->CurrentToken = ExpectToken(State->CurrentToken, State, TOKEN_END);
        return Declaration(Name, Init, Type);
}

EXPRESSION *ParseStruct(ArborState *State)
{
        EXPRESSION *Body = NULL;
        char *Name = NULL;
        if (State->CurrentToken.Type != TOKEN_IDENTIFIER)
        {
                TODO("Errors -- ParseStruct.Ident");
                return NULL;
        }
        Name = strdup(State->CurrentToken.Identifier);
        State->CurrentToken = GetToken(State);
        Body = ParseStatement(State);
        return Structure(Name, Body);
}

EXPRESSION *ParseReturn(ArborState *State)
{
        return Return(ParseExpression(State));
}

EXPRESSION *ParseStatement(ArborState *State)
{
        EXPRESSION *Expr = NULL, *Sub;
        switch (State->CurrentToken.Type)
        {
        case TOKEN_RETURN:
                State->CurrentToken = GetToken(State);
                Expr = ParseReturn(State);
                if (State->CurrentToken.Type != TOKEN_END)
                {
                        printf("Expected end of statement, got token type: %d\n",
                               State->CurrentToken.Type);
                }
                State->CurrentToken = GetToken(State);
                break;
        case TOKEN_STRUCT:
                State->CurrentToken = GetToken(State);
                Expr = ParseStruct(State);
                break;
        case TOKEN_IF:
                State->CurrentToken = GetToken(State);
                Expr = ParseIf(State);
                break;
        case TOKEN_FN:
                State->CurrentToken = GetToken(State);
                Expr = ParseFunction(State);
                break;
        case TOKEN_LET:
                State->CurrentToken = GetToken(State);
                Expr = ParseDeclaration(State);
                break;
        case TOKEN_EXPR_LCPAREN:
                State->CurrentToken = GetToken(State);
                while (State->CurrentToken.Type != TOKEN_NONE && State->CurrentToken.Type != TOKEN_EXPR_RCPAREN)
                {
                        Sub = ParseStatement(State);
                        AppendExpr(&Expr, Sub);
                }
                State->CurrentToken = GetToken(State);
                break;
        default:
                Expr = ParseAssignment(State);
                if (State->CurrentToken.Type != TOKEN_END)
                {
                        printf("Expected end of statement, got token type: %d\n",
                               State->CurrentToken.Type);
                }
                State->CurrentToken = GetToken(State);
                break;
        }

        return (Expr);
}

EXPRESSION *ParseStatements(ArborState *State)
{
        EXPRESSION *Expressions = NULL;

        if (!ValidateState(State) ||
            !State->Assembly)
        {
                return NULL;
        }

        while (State->CurrentToken.Type != TOKEN_NONE)
        {
                EXPRESSION *Expr = NULL;
                Expr = ParseStatement(State);

                if (Expr)
                {
                        AppendExpr(&Expressions, Expr);
                }
        }

        return (Expressions);
}

void DisplayExpressionTree(EXPRESSION *Expr, int Depth)
{
        EXPRESSION *Stmt;
        if (!Expr)
                return;

        for (int i = 0; i < Depth; i++)
                printf("  ");

        switch (Expr->Type)
        {
        case EXPR_TYPE_ACCESS:
                printf("Index:\n");
                DisplayExpressionTree(Expr->as.access.Index, Depth + 1);
                for (int i = 0; i < Depth; i++)
                        printf("  ");
                printf("Into:\n");
                DisplayExpressionTree(Expr->as.access.Expr, Depth + 1);
                break;
        case EXPR_TYPE_DECLARATION:
                printf("Declare, ");
                printf("%s ", Expr->as.declaration.Name);
                for (SIZE i = 0; i < Expr->as.declaration.Type.Variant.Depth; ++i)
                {
                        printf("*");
                }
                for (SIZE i = 0; i < Expr->as.declaration.Type.Variant.DimCount; ++i)
                {
                        printf("[%zu]", Expr->as.declaration.Type.Variant.Dim[i]);
                }

                if (Expr->as.declaration.Type.Constant)
                        printf("const ");
                if (!Expr->as.declaration.Type.IsStructure)
                {
                        printf("%c%zu\n",
                               Expr->as.declaration.Type.as.normal.Signed ? 'i' : 'u',
                               Expr->as.declaration.Type.as.normal.Bits);
                }
                else if (Expr->as.declaration.Type.as.structure.StructureName)
                        printf("%s\n", Expr->as.declaration.Type.as.structure.StructureName);
                DisplayExpressionTree(Expr->as.declaration.Init, Depth + 1);
                break;
        case EXPR_TYPE_CALL:
                printf("Call, %zu arguments:\n", Expr->as.call.ArgCount);
                DisplayExpressionTree(Expr->as.call.Callee, Depth + 1);
                DisplayExpressionTree(Expr->as.call.Args, Depth + 1);
                break;

        case EXPR_TYPE_UNARY_OP:
                printf("Unary, type %d\n", Expr->as.unary.Operator);
                DisplayExpressionTree(Expr->as.unary.Operand, Depth + 1);
                break;

        case EXPR_TYPE_NONE:
                printf("<None>\n");
                break;

        case EXPR_TYPE_LITERAL_STR:
                printf("Literal: '%s'\n", Expr->as.string_literal.Data);
                break;
        case EXPR_TYPE_LITERAL_NUM:
                printf("Literal: %zu\n", Expr->as.integer_literal.Value);
                break;
        case EXPR_TYPE_LITERAL_REAL:
                printf("Literal: %lf\n", Expr->as.real_literal.Value);
                break;

        case EXPR_TYPE_VAR:
                printf("Variable: %s\n", Expr->as.variable.Name);
                break;

        case EXPR_TYPE_BINARY_OP:
        {
                char *op_str = "?";
                switch (Expr->as.binary.Operator)
                {
                case TOKEN_EXPR_ADD:
                        op_str = "+";
                        break;
                case TOKEN_EXPR_SUB:
                        op_str = "-";
                        break;
                case TOKEN_EXPR_MUL:
                        op_str = "*";
                        break;
                case TOKEN_EXPR_DIV:
                        op_str = "/";
                        break;
                default:
                        op_str = "?";
                        break;
                }
                printf("BinaryOp: %s\n", op_str);
                DisplayExpressionTree(Expr->as.binary.Lhs, Depth + 1);
                DisplayExpressionTree(Expr->as.binary.Rhs, Depth + 1);
                break;
        }

        case EXPR_TYPE_ASSIGNMENT:
                printf("Assignment:\n");
                DisplayExpressionTree(Expr->as.assignment.Lhs, Depth + 1);
                DisplayExpressionTree(Expr->as.assignment.Rhs, Depth + 1);
                break;

        case EXPR_TYPE_IFELSE:
                printf("If-Else, Condition:\n");
                DisplayExpressionTree(Expr->as.ifelse.Conditional, Depth + 2);
                printf("\n");
                for (int i = 0; i < Depth; i++)
                        printf("  ");
                printf("If:\n");
                DisplayExpressionTree(Expr->as.ifelse.Body, Depth + 2);
                printf("\n");
                if (Expr->as.ifelse.ElseBody)
                {
                        for (int i = 0; i < Depth; i++)
                                printf("  ");
                        printf("Else:\n");
                        DisplayExpressionTree(Expr->as.ifelse.ElseBody, Depth + 2);
                }
                break;
        case EXPR_TYPE_STRUCTURE:
                printf("Structure: %s\n", Expr->as.structure.Name);
                DisplayExpressionTree(Expr->as.structure.Body, Depth + 1);
                break;
        case EXPR_TYPE_FUNCTION:
                printf("Function: %s\n", Expr->as.fun.Name);
                printf("Parameters: ");
                DisplayExpressionTree(Expr->as.fun.Params, Depth + 1);
                printf("Body:\n");
                DisplayExpressionTree(Expr->as.fun.Body, Depth + 1);
                break;

        default:
                printf("Unknown Expression Type: %d\n", Expr->Type);
                break;
        }

        if (Expr->Next)
        {
                DisplayExpressionTree(Expr->Next, Depth);
        }
}
