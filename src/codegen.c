#include "codegen.h"

STRUCTURE *FindStructureInScope(SCOPE *Scope, TYPE Type)
{
	if (!Type.IsStructure)
		return NULL;
	STRUCTURE *Struct = Scope->DataTypes;
	while (Struct)
	{
		if (!strncmp(Struct->Name, Type.as.structure.StructureName, 32))
		{
			return Struct;
		}
		Struct = Struct->Next;
	}

	return NULL;
}

STRUCTURE *FindStructure(ArborState *State, TYPE Type)
{
	if (!Type.IsStructure)
		return NULL;
	SCOPE *Scope = State->TranslationUnit.CurrentScope;
	while (Scope)
	{
		STRUCTURE *Struct = FindStructureInScope(Scope, Type);
		if (Struct)
			return Struct;
		Scope = Scope->Parent;
	}

	return NULL;
}

VARIABLE *NewVariable(ArborState *State, TYPE Type, char *Name, BOOL IsOffset, int Address)
{
	SCOPE *Scope = State->TranslationUnit.CurrentScope;
	VARIABLE *Variable = new (VARIABLE);
	Variable->Type = Type;
	Variable->Name = Name;
	Variable->IsOffset = IsOffset;
	Variable->Address = Address;

	if (Type.IsStructure)
		return Variable;
	size_t Bytes = Type.Variant.Depth > 0 ? 8 : Type.as.normal.Bits / 8;
	for (size_t i = 0; i < Type.Variant.DimCount; ++i)
	{
		Bytes *= Type.Variant.Dim[i];
	}
	Variable->SizeOf = Bytes;
	ll_append(&Scope->Vars, Variable);

	State->TranslationUnit.CurrentScope->BpOff -= Bytes;
	return (Variable);
}

static VARIABLE *FindVar(ArborState *State, EXPRESSION *Expr, const char *Name, size_t *Depth)
{
	if (!Name || !State->TranslationUnit.CurrentScope)
	{
		return NULL;
	}
	else if (Expr && Expr->Type != EXPR_TYPE_FUNCTION)
	{
		return FindVar(State, Expr->Parent, Name, Depth);
	}

	VARIABLE *Var = State->TranslationUnit.CurrentScope->Vars;
	while (Var)
	{
		if (!strncmp(Name, Var->Name, 64))
		{
			return Var;
		}
		Var = Var->Next;
	}

	(*Depth)++;
	if (Expr)
		return FindVar(State, Expr->Parent, Name, Depth);
	return NULL;
}

static void GenerateBinary(ArborState *State, EXPRESSION *Expr)
{
	GenerateForExpr(State, Expr->as.binary.Lhs);
	switch (Expr->as.binary.Operator)
	{
	case TOKEN_EXPR_CHAIN_OR:
	case TOKEN_EXPR_CHAIN_AND:
	default:
		break;
	}
	GenerateForExpr(State, Expr->as.binary.Rhs);
	fprintf(State->Output, "\tpop eax\n");

	switch (Expr->as.binary.Operator)
	{
	case TOKEN_EXPR_ADD:
		fprintf(State->Output, "\tadd [esp], eax\n");
		break;
	case TOKEN_EXPR_SUB:
		fprintf(State->Output, "\tsub [esp], eax\n");
		break;
	case TOKEN_EXPR_MUL:
		fprintf(State->Output, "\tpop ebx\n");
		fprintf(State->Output, "\timul ebx\n");
		fprintf(State->Output, "\tpush eax\n");
		break;
	case TOKEN_EXPR_DIV:
		fprintf(State->Output, "\tpop ebx\n");
		fprintf(State->Output, "\txchg eax, ebx\n");
		fprintf(State->Output, "\tcdq\n");
		fprintf(State->Output, "\tidiv ebx\n");
		fprintf(State->Output, "\tpush eax\n");
		break;
	case TOKEN_EXPR_MOD:
		fprintf(State->Output, "\tpop ebx\n");
		fprintf(State->Output, "\txchg eax, ebx\n");
		fprintf(State->Output, "\tcdq\n");
		fprintf(State->Output, "\tidiv ebx\n");
		fprintf(State->Output, "\tpush edx\n");
		break;
	case TOKEN_EXPR_EQEQ:
		fprintf(State->Output, "\tcmp [esp], eax\n");
		fprintf(State->Output, "\tsete al\n");
		fprintf(State->Output, "\tmovzx eax, al\n");
		fprintf(State->Output, "\tmov [esp], eax\n");
		break;
	case TOKEN_EXPR_LESS:
		fprintf(State->Output, "\tcmp [esp], eax\n");
		fprintf(State->Output, "\tsetl al\n");
		fprintf(State->Output, "\tmovzx eax, al\n");
		fprintf(State->Output, "\tmov [esp], eax\n");
		break;
	case TOKEN_EXPR_GREATER:
		fprintf(State->Output, "\tcmp [esp], eax\n");
		fprintf(State->Output, "\tsetg al\n");
		fprintf(State->Output, "\tmovzx eax, al\n");
		fprintf(State->Output, "\tmov [esp], eax\n");
		break;
	case TOKEN_EXPR_AND:
		fprintf(State->Output, "\tand [esp], eax\n");
		break;
	case TOKEN_EXPR_OR:
		fprintf(State->Output, "\tor [esp], eax\n");
		break;
	case TOKEN_EXPR_XOR:
		fprintf(State->Output, "\txor [esp], eax\n");
		break;
	default:
		fprintf(State->Output, "\t; Unknown operator\n");
		fprintf(State->Output, "\tadd esp, 4\n");
		break;
	}
}

static void AppendSuffix(char *buff, size_t n, TYPE Type)
{
	if (!buff)
		return;

	size_t used = strlen(buff);
	size_t remaining = n - used;

	if (remaining <= 1)
		return;

	snprintf(buff + used, remaining, "_");
	used = strlen(buff);
	remaining = n - used;

	for (size_t i = 0; i < Type.Variant.Depth && remaining > 1; ++i)
	{
		snprintf(buff + used, remaining, "p_");
		used = strlen(buff);
		remaining = n - used;
	}

	for (size_t i = 0; i < Type.Variant.DimCount && remaining > 1; ++i)
	{
		snprintf(buff + used, remaining, "d%zu_", Type.Variant.Dim[i]);
		used = strlen(buff);
		remaining = n - used;
	}

	if (Type.Constant && remaining > 7)
	{
		snprintf(buff + used, remaining, "const_");
		used = strlen(buff);
		remaining = n - used;
	}

	if (remaining > 1)
	{
		if (Type.IsStructure)
		{
			snprintf(buff + used, remaining, "%s", Type.as.structure.StructureName);
		}
		else
		{
			snprintf(buff + used, remaining, "%c%zu",
				 Type.as.normal.Signed ? 'i' : 'u',
				 Type.as.normal.Bits);
		}
	}
}

static char *FindSuffixForType(TYPE Type)
{
	char *buff = calloc(1, 1024);
	AppendSuffix(buff, 1024, Type);
	return buff;
}

static char *FindSuffixesForFunction(EXPRESSION *Expr)
{
	if (!Expr || (Expr->Type != EXPR_TYPE_FUNCTION && Expr->Type != EXPR_TYPE_EXTERN))
		return NULL;
	char *Suffix = calloc(1, 1);

	EXPRESSION *Param = Expr->Type != EXPR_TYPE_EXTERN ? Expr->as.fun.Params : Expr->as.ext.Params;
	while (Param)
	{
		AppendSuffix(Suffix, 1024, Param->as.declaration.Type);
		Param = Param->Next;
	}

	return Suffix;
}

static void EmitFunction(ArborState *State, EXPRESSION *Expr)
{
	char *Suffix = NULL;
	Suffix = FindSuffixesForFunction(Expr);
	fprintf(State->Output, "\tjmp __Arbor_%s%s.end\n", Expr->as.fun.Name, Suffix);
	fprintf(State->Output, "__Arbor_%s%s:\n\tpush ebp\n\tpush esi\n\tmov esi, ebp\n\tmov ebp, esp\n", Expr->as.fun.Name, Suffix);

	size_t Count = 0;
	// VARIABLE *Vars = DeclareVariables(State, Expr, &Count);
	// if (Count > 0)
	//         fprintf(State->Output, "\tsub esp, %zu\n", Count);
	// Expr->as.fun.Vars = Vars;
	SCOPE *Scope = new (SCOPE);
	ll_append_child(&State->TranslationUnit.CurrentScope, Scope);
	State->TranslationUnit.CurrentScope = Scope;

	TYPE ptrType = {
	    .IsStructure = FALSE,
	    .as.normal.Bits = 32,
	    .as.normal.Signed = FALSE,
	    .Variant = {0},
	    .Constant = TRUE};

	VARIABLE *Fn = NewVariable(State, Expr->as.fun.ReturnType, Expr->as.fun.Name, FALSE, 0);
	Scope->BpOff = 12;
	VARIABLE *oldsi = NewVariable(State, ptrType, "__oldsi", TRUE, 0);
	VARIABLE *oldbp = NewVariable(State, ptrType, "__oldbp", TRUE, 4);
	VARIABLE *retadr = NewVariable(State, ptrType, "__retadr", TRUE, 8);
	ll_append(&Scope->Vars, Fn);
	ll_append(&Scope->Vars, oldsi);
	ll_append(&Scope->Vars, oldbp);
	ll_append(&Scope->Vars, retadr);

	Fn->IsFunction = TRUE;
	Fn->Expr = Expr;

	GenerateForExpr(State, Expr->as.fun.Body);
	State->TranslationUnit.CurrentScope = State->TranslationUnit.CurrentScope->Parent;
	fprintf(State->Output, "__Arbor_%s%s.end:\n\tmov esp, ebp\n\tpop esi\n\tpop ebp\n\tret\n", Expr->as.fun.Name, Suffix);
	free(Suffix);
}

void EmitStart(ArborState *State)
{
	if (!State || !State->Output)
		return;
	fprintf(State->Output, "\tglobal main\n");
	fprintf(State->Output, "\textern __Arbor_main_i32_p_p_i8\n");
	fprintf(State->Output, "\textern __Arbor_print_p_i8\n");
	fprintf(State->Output, "\tbits 32\n");
	fprintf(State->Output, "main:\tjmp __Arbor_main_i32_p_p_i8\n");
}

TYPE EmitVar(ArborState *State, EXPRESSION *Expr)
{
	size_t Depth = 0, ADepth;
	VARIABLE *Var = FindVar(State, Expr, Expr->as.variable.Name, &Depth);
	ADepth = Depth;
	if (!Var)
	{
		printf("Unknown Variable: %s\n", Expr->as.variable.Name);
		return (TYPE){0};
	}

	printf("Found %s at depth of %ld\n", Expr->as.variable.Name, Depth);
	if (ADepth > 1)
		fprintf(State->Output, "\tmov ebx, esi\n");
	while (Depth > 1)
	{
		fprintf(State->Output, "\tmov esi, [esi]\n");
		Depth -= 1;
	}

	if (Var->IsFunction)
	{
		char *Suffix = NULL;
		Suffix = FindSuffixesForFunction(Var->Expr);
		fprintf(State->Output, "\tpush __Arbor_%s%s\n", Var->Name, Suffix);
		free(Suffix);
	}
	else if (Var->Type.IsStructure)
	{
		/* .. */
	}
	else if (Var->Type.as.normal.Bits <= 32)
	{
		if (Depth == 0)
			fprintf(State->Output, "\tpush dword[ebp+%ld]\n", Var->Address);
		else if (Depth == 1)
			fprintf(State->Output, "\tpush dword[esi+%ld]\n", Var->Address);
	}
	if (ADepth > 1)
		fprintf(State->Output, "\tmov esi, ebx\n");
	return Var->Type;
}

TYPE EmitCall(ArborState *State, EXPRESSION *Expr)
{
	fprintf(State->Output, "\tpush ebx\n");
	fprintf(State->Output, "\tmov ebx, esp\n");
	GenerateForExpr(State, Expr->as.call.Args);
	TYPE Type = GenerateForExpr(State, Expr->as.call.Callee);
	fprintf(State->Output, "\tpop eax\n");
	fprintf(State->Output, "\tcall eax\n");
	fprintf(State->Output, "\tmov esp, ebx\n");
	fprintf(State->Output, "\tpop ebx\n");
	fprintf(State->Output, "\tpush eax\n");
	return Type;
}

void EmitAssignment(ArborState *State, EXPRESSION *Expr)
{
	GenerateForExpr(State, Expr->as.assignment.Rhs);
	size_t Depth = 0, ADepth;
	VARIABLE *Var = FindVar(State, Expr, Expr->as.assignment.Lhs->as.variable.Name, &Depth);
	ADepth = Depth;
	if (!Var)
	{
		printf("Unknown Variable: %s\n", Expr->as.assignment.Lhs->as.variable.Name);
		return;
	}

	printf("Found %s at depth of %ld\n", Expr->as.assignment.Lhs->as.variable.Name, Depth);
	if (ADepth > 1)
		fprintf(State->Output, "\tmov ebx, esi\n");
	while (Depth > 1)
	{
		fprintf(State->Output, "\tmov esi, [esi]\n");
		Depth -= 1;
	}

	if (Var->IsFunction)
	{
		/* .. */
	}
	else if (Var->Type.IsStructure)
	{
		/* .. */
	}
	else if (Var->Type.as.normal.Bits <= 32)
	{
		if (Depth == 0)
			fprintf(State->Output, "\tpop dword[ebp+%ld]\n", Var->Address);
		else if (Depth == 1)
			fprintf(State->Output, "\tpop dword[esi+%ld]\n", Var->Address);
	}
	if (ADepth > 1)
		fprintf(State->Output, "\tmov esi, ebx\n");
}

TYPE EmitStringLiteral(ArborState *State, EXPRESSION *Expr)
{
	TYPE Type = {0};
	TSTRING *String = new (TSTRING);
	String->CStr = Expr->as.string_literal.Data;
	String->Length = Expr->as.string_literal.Value;
	String->Parent = NULL;
	String->Child = NULL;
	Type.IsStructure = FALSE;
	Type.Variant.Depth = 0;
	Type.Variant.Dim[0] = Expr->as.string_literal.Value;
	Type.Variant.DimCount = 1;
	ll_append(&State->TranslationUnit.StringTable, String);
	fprintf(State->Output, "\tpush __string_%ld\n", State->TranslationUnit.StringCount++);
	return Type;
}

void EmitStruct(ArborState *State, EXPRESSION *Expr)
{
	STRUCTURE *Structure = new (STRUCTURE);
	Structure->Body = Expr->as.structure.Body;
	Structure->Name = Expr->as.structure.Name;
	ll_append(&State->TranslationUnit.CurrentScope->DataTypes, Structure);
}

void EmitDeclaration(ArborState *State, EXPRESSION *Expr)
{
	TYPE Type = {0};
	if (Expr->as.declaration.Init)
	{
		Type = GenerateForExpr(State, Expr->as.declaration.Init);
		long error = memcmp(&Type, &Expr->as.declaration.Type, sizeof(TYPE));
		if (error)
		{
			printf("ERROR: Types differ by %ld\n", error);
		}
	}

	VARIABLE *Var = NewVariable(State, Expr->as.declaration.Type, Expr->as.declaration.Name, TRUE, State->TranslationUnit.CurrentScope->BpOff);
}

void EmitExtern(ArborState *State, EXPRESSION *Expr)
{
	VARIABLE *Var = new (VARIABLE);
	Var->Type = Expr->as.ext.ReturnType;
	Var->IsFunction = TRUE;
	ll_append(&State->TranslationUnit.CurrentScope->Vars, Var);

	char *Suffix = NULL;
	Suffix = FindSuffixesForFunction(Expr);
	fprintf(State->Output, "\textern __Arbor_%s%s\n", Expr->as.ext.Name, Suffix);
	free(Suffix);
}

TYPE GenerateForExpr(ArborState *State, EXPRESSION *Expr)
{
	TYPE Type = {0};
	if (!Expr)
		return Type;
	printf("EXPR=%p,%d\n", (void *)Expr, Expr->Type);
	switch (Expr->Type)
	{
	case EXPR_TYPE_DECLARATION:
		EmitDeclaration(State, Expr);
		break;
	case EXPR_TYPE_ASSIGNMENT:
		EmitAssignment(State, Expr);
		break;
	case EXPR_TYPE_VAR:
		Type = EmitVar(State, Expr);
		break;
	case EXPR_TYPE_LITERAL_NUM:
		fprintf(State->Output, "\tpush %ld\n", Expr->as.integer_literal.Value);
		Type.as.normal.Bits = 32;
		Type.as.normal.Signed = TRUE;
		break;
	case EXPR_TYPE_BINARY_OP:
		GenerateBinary(State, Expr);
		break;
	case EXPR_TYPE_FUNCTION:
		EmitFunction(State, Expr);
		break;
	case EXPR_TYPE_EXTERN:
		EmitExtern(State, Expr);
		break;
	case EXPR_TYPE_CALL:
		Type = EmitCall(State, Expr);
		break;
	case EXPR_TYPE_RETURN:
		GenerateForExpr(State, Expr->as.return_statement);
		fprintf(State->Output, "\tpop eax\n");
		fprintf(State->Output, "\tjmp .end\n");
		break;
	case EXPR_TYPE_LITERAL_STR:
		Type = EmitStringLiteral(State, Expr);
		break;
	case EXPR_TYPE_STRUCTURE:
		EmitStruct(State, Expr);
		break;
	default:
		break;
	}
	GenerateForExpr(State, Expr->Next);
	return Type;
}
