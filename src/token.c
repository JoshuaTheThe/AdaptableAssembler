#include"types.h"
#include"token.h"
#include"state.h"

static const STRING Keywords[] =
{
        "let",
        "fn",
        "if",
        "else",
        "while",
        "for",
};

static TOKEN GetNumber(ArborState *State)
{
	TOKEN Token = {0};
	int chr;
	if (!ValidateState(State)||
	    !State->Assembly)
	{
		return(Token);
	}
	
	Token.Type = TOKEN_NUMBER;
        Token.Number = 0;
	chr = getc(State->Assembly);
	while (isdigit(chr))
	{
		Token.Number =
			Token.Number * 10 +
			((SIZE)chr - '0');
		chr = getc(State->Assembly);
	}
	ungetc(chr, State->Assembly);

	return(Token);
}

static TOKEN GetIdent(ArborState *State)
{
        TOKEN Token = {0};
        SIZE Len = 0;
	int chr;
	if (!ValidateState(State)||
	    !State->Assembly)
	{
		return(Token);
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

        return(Token);
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
        else if (chr == '+') { Token.Type = TOKEN_EXPR_ADD; }
        else if (chr == '-') { Token.Type = TOKEN_EXPR_SUB; }
        else if (chr == '*') { Token.Type = TOKEN_EXPR_MUL; }
        else if (chr == '/') { Token.Type = TOKEN_EXPR_DIV; }
        else if (chr == '&') { Token.Type = TOKEN_EXPR_AND; }
        else if (chr == '|') { Token.Type = TOKEN_EXPR_OR; }
        else if (chr == '^') { Token.Type = TOKEN_EXPR_XOR; }
        else if (chr == '(') { Token.Type = TOKEN_EXPR_LPAREN; }
        else if (chr == ')') { Token.Type = TOKEN_EXPR_RPAREN; }
        else if (chr == '{') { Token.Type = TOKEN_EXPR_LCPAREN; }
        else if (chr == '}') { Token.Type = TOKEN_EXPR_RCPAREN; }
        else if (chr == '=') { Token.Type = TOKEN_EXPR_EQ; }
        else if (chr == ';') { Token.Type = TOKEN_END; }
        else if (chr == ',') { Token.Type = TOKEN_EXPR_COMMA; }
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
                        for (SIZE i = 0; i < sizeof(Keywords)/sizeof(*Keywords); ++i)
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
	return(Token);
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
                (*Success)=TRUE;
                return GetToken(State);
        }
end:
        (*Success)=FALSE;
        return(TOKEN){0};
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
                return(TRUE);
        }
end:
        return(FALSE);
}

TOKEN ExpectToken(TOKEN Current, ArborState *State, TOKENTYPE Type)
{
        BOOL Success;
        TOKEN Next;
        if (!ValidateState(State) ||
	    !State->Assembly ||
	    !State->Arch)
	{
		goto end;
	}

        Next = AcceptToken(Current, State, Type, &Success);
        if (Success)
        {
                return(Next);
        }
end:
        return(TOKEN){0};
}
