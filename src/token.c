#include"types.h"
#include"token.h"
#include"state.h"

static TOKEN GetNumber(AASState *State)
{
	TOKEN Token = {0};
	int chr;
	if (!ValidateState(State)||
	    !State->Assembly)
	{
		return Token;
	}
	
	Token.Type = TOKEN_NUMBER;
	chr = getc(State->Assembly);
	while (isdigit(chr))
	{
		Token.Number =
			Token.Number * 10 +
			(SIZE)chr - '0';
		chr = getc(State->Assembly);
	}
	ungetc(chr, State->Assembly);

	return Token;
}

TOKEN GetToken(AASState *State)
{
	TOKEN Token = {0};
	int chr;
	if (!ValidateState(State) ||
	    !State->Assembly ||
	    !State->Arch)
	{
		goto end;
	}

	chr = getc(State->Assembly);
	if (chr == EOF)
	{
		goto end;
	}
	else if (isdigit(chr))
	{
		ungetc(chr, State->Assembly);
		Token = GetNumber(State);
	}
end:
	return(Token);
}

