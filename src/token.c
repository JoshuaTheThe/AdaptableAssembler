#include "types.h"
#include "token.h"
#include "state.h"

static const STRING Keywords[] =
    {
        "let",
        "fn",
        "if",
        "else",
        "while",
        "for",
        "int",
        "int16",
        "int8",
        "real",
        "unsigned",
        "const",
        "struct",
        "return",
};

static TOKEN GetNumber(ArborState *State)
{
        TOKEN Token = {0};
        int chr;
        int has_decimal = 0;
        int has_exponent = 0;
        int exponent_sign = 1;
        double fraction = 0.1;
        double exponent = 0;

        if (!ValidateState(State) || !State->Assembly)
        {
                return Token;
        }

        int is_negative = 0;
        chr = getc(State->Assembly);

        if (chr == '-')
        {
                is_negative = 1;
                chr = getc(State->Assembly);
        }
        else if (chr == '+')
        {
                chr = getc(State->Assembly);
        }

        if (!isdigit(chr))
        {
                ungetc(chr, State->Assembly);
                if (is_negative)
                        ungetc('-', State->Assembly);
                Token.Type = TOKEN_NONE;
                return Token;
        }

        Token.Type = TOKEN_NUMBER;
        Token.Number = 0;
        double real_value = 0.0;

        while (isdigit(chr))
        {
                Token.Number = Token.Number * 10 + (chr - '0');
                real_value = real_value * 10.0 + (chr - '0');
                chr = getc(State->Assembly);
        }

        if (chr == '.')
        {
                Token.Type = TOKEN_REAL;
                has_decimal = 1;
                chr = getc(State->Assembly);

                while (isdigit(chr))
                {
                        real_value += (chr - '0') * fraction;
                        fraction *= 0.1;
                        chr = getc(State->Assembly);
                }
        }

        if (chr == 'e' || chr == 'E')
        {
                Token.Type = TOKEN_REAL;
                has_exponent = 1;
                chr = getc(State->Assembly);

                if (chr == '-')
                {
                        exponent_sign = -1;
                        chr = getc(State->Assembly);
                }
                else if (chr == '+')
                {
                        exponent_sign = 1;
                        chr = getc(State->Assembly);
                }

                while (isdigit(chr))
                {
                        exponent = exponent * 10 + (chr - '0');
                        chr = getc(State->Assembly);
                }

                if (exponent > 0)
                {
                        double multiplier = 1.0;
                        for (int i = 0; i < exponent; i++)
                        {
                                multiplier *= (exponent_sign > 0 ? 10.0 : 0.1);
                        }
                        real_value *= multiplier;
                }
        }

        if (chr != EOF)
        {
                ungetc(chr, State->Assembly);
        }

        if (is_negative)
        {
                Token.Number = -Token.Number;
                real_value = -real_value;
        }

        if (Token.Type == TOKEN_REAL || has_exponent)
        {
                Token.Type = TOKEN_REAL;
                Token.Real = real_value;
        }
        else
        {
                Token.Number = (long long)real_value;
        }

        return Token;
}

static TOKEN GetIdent(ArborState *State)
{
        TOKEN Token = {0};
        SIZE Len = 0;
        int chr;
        if (!ValidateState(State) ||
            !State->Assembly)
        {
                return (Token);
        }
        Token.Type = TOKEN_IDENTIFIER;
        Token.Identifier = NULL;
        chr = getc(State->Assembly);
        while (isdigit(chr) || isalpha(chr) || chr == '_')
        {
                Token.Identifier = realloc(Token.Identifier, Len + 2);
                if (!Token.Identifier)
                {
                        printf("Failed to reallocate\n");
                        return (TOKEN){0};
                }
                Token.Identifier[Len] = (char)chr;
                Token.Identifier[++Len] = (char)0;
                chr = getc(State->Assembly);
        }
        ungetc(chr, State->Assembly);

        return (Token);
}

static TOKEN GetString(ArborState *State)
{
        TOKEN Token = {0};
        SIZE Len = 0;
        int chr;
        if (!ValidateState(State) ||
            !State->Assembly)
        {
                return (Token);
        }
        Token.Type = TOKEN_STRING;
        Token.Identifier = NULL;
        chr = getc(State->Assembly);
        _assert(chr == '"');
        chr = getc(State->Assembly);
        while (chr != '"')
        {
                if (chr == '\\')
                {
                        chr = getc(State->Assembly);
                        switch (chr)
                        {
                        case 't':
                                chr = '\t';
                                break;
                        case 'a':
                                chr = '\a';
                                break;
                        case 'v':
                                chr = '\v';
                                break;
                        case 'n':
                                chr = '\n';
                                break;
                        case 'r':
                                chr = '\r';
                                break;
                        default:
                                break;
                        }
                }
                Token.Identifier = realloc(Token.Identifier, Len + 2);
                if (!Token.Identifier)
                {
                        printf("Failed to reallocate\n");
                        return (TOKEN){0};
                }
                Token.Identifier[Len] = (char)chr;
                Token.Identifier[++Len] = (char)0;
                chr = getc(State->Assembly);
        }

        return (Token);
}

TOKEN GetToken(ArborState *State)
{
        TOKEN Token = {0};
        int chr;
        if (!ValidateState(State) ||
            !State->Assembly ||
            !State->Arch)
        {
                goto end;
        }

        FreeToken(&State->CurrentToken);

        do
        {
                chr = getc(State->Assembly);
        } while (chr == ' ' || chr == '\t' || chr == '\n');
        if (chr == EOF)
        {
                goto end;
        }
        else if (chr == '+')
        {
                Token.Type = TOKEN_EXPR_ADD;
        }
        else if (chr == '-')
        {
                Token.Type = TOKEN_EXPR_SUB;
        }
        else if (chr == '*')
        {
                Token.Type = TOKEN_EXPR_MUL;
        }
        else if (chr == '/')
        {
                Token.Type = TOKEN_EXPR_DIV;
        }
        else if (chr == '&')
        {
                Token.Type = TOKEN_EXPR_AND;
        }
        else if (chr == '|')
        {
                Token.Type = TOKEN_EXPR_OR;
        }
        else if (chr == '^')
        {
                Token.Type = TOKEN_EXPR_XOR;
        }
        else if (chr == '(')
        {
                Token.Type = TOKEN_EXPR_LPAREN;
        }
        else if (chr == ')')
        {
                Token.Type = TOKEN_EXPR_RPAREN;
        }
        else if (chr == '{')
        {
                Token.Type = TOKEN_EXPR_LCPAREN;
        }
        else if (chr == '}')
        {
                Token.Type = TOKEN_EXPR_RCPAREN;
        }
        else if (chr == '=')
        {
                Token.Type = TOKEN_EXPR_EQ;
        }
        else if (chr == ';')
        {
                Token.Type = TOKEN_END;
        }
        else if (chr == ',')
        {
                Token.Type = TOKEN_EXPR_COMMA;
        }
        else if (chr == '!')
        {
                Token.Type = TOKEN_EXPR_NOT;
        }
        else if (chr == '~')
        {
                Token.Type = TOKEN_EXPR_COMPLEMENT;
        }
        else if (chr == '[')
        {
                Token.Type = TOKEN_EXPR_LSPAREN;
        }
        else if (chr == ']')
        {
                Token.Type = TOKEN_EXPR_RSPAREN;
        }
        else if (chr == '"')
        {
                ungetc(chr, State->Assembly);
                Token = GetString(State);
        }
        else if (isdigit(chr))
        {
                ungetc(chr, State->Assembly);
                Token = GetNumber(State);
        }
        else if (isalpha(chr) || chr == '_')
        {
                ungetc(chr, State->Assembly);
                Token = GetIdent(State);
                chr = getc(State->Assembly);
                if (chr == ':')
                {
                        Token.Type = TOKEN_LABEL;
                }
                else
                {
                        ungetc(chr, State->Assembly);
                        for (SIZE i = 0; i < sizeof(Keywords) / sizeof(*Keywords); ++i)
                        {
                                if (!strncmp(Keywords[i], Token.Identifier, MAX_INST_LENGTH))
                                {
                                        Token.Type = TOKEN_KEYWORDS + (TOKENTYPE)i;
                                        break;
                                }
                        }
                }
        }
end:
        return (Token);
}

void FreeToken(TOKEN *Token)
{
        if (Token->Type == TOKEN_IDENTIFIER ||
            Token->Type == TOKEN_LABEL ||
            Token->Type >= TOKEN_KEYWORDS)
        {
                free(Token->Identifier);
        }
}

TOKEN AcceptToken(TOKEN Current, ArborState *State, TOKENTYPE Type, BOOL *Success)
{
        if (!ValidateState(State) ||
            !State->Assembly ||
            !State->Arch)
        {
                goto end;
        }

        if (Current.Type == Type)
        {
                (*Success) = TRUE;
                return GetToken(State);
        }
end:
        (*Success) = FALSE;
        return (TOKEN){0};
}

BOOL AcceptsToken(TOKEN Current, ArborState *State, TOKENTYPE Type)
{
        if (!ValidateState(State) ||
            !State->Assembly ||
            !State->Arch)
        {
                goto end;
        }

        if (Current.Type == Type)
        {
                return (TRUE);
        }
end:
        return (FALSE);
}

TOKEN ExpectToken(TOKEN Current, ArborState *State, TOKENTYPE Type)
{
        BOOL Success;
        TOKEN Next;
        if (!ValidateState(State) ||
            !State->Assembly)
        {
                goto end;
        }

        Next = AcceptToken(Current, State, Type, &Success);
        if (Success)
        {
                return (Next);
        }
end:
        return (TOKEN){0};
}
