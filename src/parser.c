#include "parser.h"

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

static EXPRESSION *Assignmment(EXPRESSION *Lhs, EXPRESSION *Rhs)
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

static void AppendExpr(AASState *State, EXPRESSION *restrict Dest, EXPRESSION *restrict Source)
{
        if (State->CurrentExpr == NULL)
        {
                State->CurrentExpr = Source;
                return;
        }

        if (Dest->Next)
                Source->Next = Dest->Next;
        Dest->Next = Source;
}

EXPRESSION *ParseFactor(AASState *State)
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
                Expr = Variable(strdup(State->CurrentToken.Identifier));
                State->CurrentToken = GetToken(State);
        }

        return Expr;
}

EXPRESSION *ParseTerm(AASState *State)
{
        EXPRESSION *Left = ParseFactor(State);

        while (State->CurrentToken.Type == TOKEN_EXPR_MUL ||
               State->CurrentToken.Type == TOKEN_EXPR_DIV)
        {
                TOKENTYPE Op = State->CurrentToken.Type;
                State->CurrentToken = GetToken(State);

                EXPRESSION *Right = ParseFactor(State);
                Left = Binary(Left, Right, Op);
        }

        return Left;
}

EXPRESSION *ParseExpression(AASState *State)
{
        EXPRESSION *Left = ParseTerm(State);

        while (State->CurrentToken.Type == TOKEN_EXPR_ADD ||
               State->CurrentToken.Type == TOKEN_EXPR_SUB)
        {
                TOKENTYPE Op = State->CurrentToken.Type;
                State->CurrentToken = GetToken(State);

                EXPRESSION *Right = ParseTerm(State);
                Left = Binary(Left, Right, Op);
        }

        return Left;
}

EXPRESSION *ParseStatements(AASState *State)
{
        if (!ValidateState(State) ||
            !State->Assembly ||
            !State->Arch)
        {
                goto end;
        }

        do
        {
                State->CurrentToken = GetToken(State);
                switch (State->CurrentToken.Type)
                {
                case TOKEN_ASSEMBLY:
                        if (State->CurrentToken.Inst && State->CurrentToken.Inst->Assemble)
                                State->CurrentToken.Inst->Assemble();
                        break;
                default:
                        DisplayExpression(ParseExpression(State));
                        State->CurrentToken = ExpectToken(State->CurrentToken, State, TOKEN_END);
                        break;
                }
                FreeToken(&State->CurrentToken);
        } while (State->CurrentToken.Type != TOKEN_NONE);
end:
        return NULL;
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

                // Print operator based on token type
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
                // Add more operators as needed
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
                // Display parameters if needed
                // For now, just display the body
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
        if (!Expr)
                return;

        // Print indentation
        for (int i = 0; i < Depth; i++)
                printf("  ");

        switch (Expr->Type)
        {
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

        case EXPR_TYPE_FUNCTION:
                printf("Function: %s\n", Expr->as.fun.Name);
                // Display parameters if you store them
                printf("Body:\n");
                DisplayExpressionTree(Expr->as.fun.Body, Depth + 1);
                break;

        default:
                printf("Unknown Expression Type: %d\n", Expr->Type);
                break;
        }
}
