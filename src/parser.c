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

static EXPRESSION *Variable(char *Name)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_VAR;
        Expr->as.var.Name = Name;
        return (Expr);
}

static EXPRESSION *Function(char *Name, EXPRESSION *Body, PARAMS Params)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_FUNCTION;
        Expr->as.fun.Name = Name;
        Expr->as.fun.Body = Body;
        Expr->as.fun.Params = Params;
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

static EXPRESSION *Call(EXPRESSION *Callee, EXPRESSION *Args)
{
        EXPRESSION *Expr = new (EXPRESSION);
        Expr->Type = EXPR_TYPE_CALL;
        Expr->as.call.Callee = Callee;
        Expr->as.call.Args = Args;
        Expr->as.call.ArgCount = Count(Args);
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
        EXPRESSION *Callee = Expr, *Arguments;
        if (State->CurrentToken.Type == TOKEN_EXPR_LPAREN)
        {
                State->CurrentToken = GetToken(State);
                Arguments = ParseArguments(State);
                State->CurrentToken = ExpectToken(State->CurrentToken, State, TOKEN_EXPR_RPAREN);
                Expr = Call(Callee, Arguments);
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
        PARAMS Params = {0};
        EXPRESSION *Body;
        State->CurrentToken = GetToken(State);
        State->CurrentToken = ExpectToken(State->CurrentToken, State, TOKEN_EXPR_LPAREN);
        State->CurrentToken = ExpectToken(State->CurrentToken, State, TOKEN_EXPR_RPAREN);
        Body = ParseStatement(State);
        return Function(Name, Body, Params);
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
        if (State->CurrentToken.Type == TOKEN_INT)
        {
                Type.IsStructure = FALSE;
                Type.as.normal.Bits = 32;
                Type.as.normal.Signed = TRUE;
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
        char *Name=NULL;
        EXPRESSION *Init=NULL;
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


EXPRESSION *ParseStatement(ArborState *State)
{
        EXPRESSION *Expr = NULL, *Sub;
        switch (State->CurrentToken.Type)
        {
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

void DisplayExpression(EXPRESSION *Expr)
{
        if (!Expr)
                return;

        switch (Expr->Type)
        {
        case EXPR_TYPE_LITERAL_NUM:
                printf("%zu", Expr->as.integer_literal.Value);
                break;

        case EXPR_TYPE_VAR:
                printf("%s", Expr->as.var.Name);
                break;

        case EXPR_TYPE_BINARY_OP:
                printf("(");
                DisplayExpression(Expr->as.binary.Lhs);

                switch (Expr->as.binary.Operator)
                {
                case TOKEN_EXPR_ADD:
                        printf(" + ");
                        break;
                case TOKEN_EXPR_SUB:
                        printf(" - ");
                        break;
                case TOKEN_EXPR_MUL:
                        printf(" * ");
                        break;
                case TOKEN_EXPR_DIV:
                        printf(" / ");
                        break;
                default:
                        printf(" ? ");
                        break;
                }

                DisplayExpression(Expr->as.binary.Rhs);
                printf(")");
                break;

        case EXPR_TYPE_ASSIGNMENT:
                DisplayExpression(Expr->as.assignment.Lhs);
                printf(" = ");
                DisplayExpression(Expr->as.assignment.Rhs);
                break;

        case EXPR_TYPE_FUNCTION:
                printf("%s(", Expr->as.fun.Name);
                DisplayExpression(Expr->as.fun.Body);
                printf(")");
                break;

        default:
                printf("<?>");
                break;
        }
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

                if (!Expr->as.declaration.Type.IsStructure)
                        printf("u%zu\n", Expr->as.declaration.Type.as.normal.Bits);
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

        case EXPR_TYPE_LITERAL_NUM:
                printf("Literal: %zu\n", Expr->as.integer_literal.Value);
                break;

        case EXPR_TYPE_VAR:
                printf("Variable: %s\n", Expr->as.var.Name);
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
        case EXPR_TYPE_FUNCTION:
                printf("Function: %s\n", Expr->as.fun.Name);

                if (Expr->as.fun.Params.Count > 0)
                {
                        for (int i = 0; i < Depth; i++)
                                printf("  ");
                        printf("Parameters: ");
                        for (SIZE i = 0; i < Expr->as.fun.Params.Count; ++i)
                        {
                                printf("%s ", Expr->as.fun.Params.Params[i]);
                        }
                }

                printf("Body:\n");

                Stmt = Expr->as.fun.Body;
                DisplayExpressionTree(Stmt, Depth + 1);
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
