#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "types.h"
#include "label.h"
#include "state.h"
#include "token.h"
#include "parser.h"

void AssembleFoo(void);

void AssembleFoo(void)
{
        printf("FOO\n");
}

void *Aalloc(SIZE sz)
{
        void *P = calloc(1, sz);
        _assert(P);
        return P;
}

void GenerateForExpr(EXPRESSION *Expr);

int main(int argc, char **argv)
{
        /* Checks and Init */
        ArborState State;
        ARCHITECTURE Arch;
        INSTRUCTION Instruction;
        EXPRESSION *Expr;
        if (argc != 3)
        {
                printf("Invalid Usage\n%s <source> <output>\n", argv[0]);
                return 1;
        }

        State = NewState();
        State.Assembly = fopen(argv[1], "r");
        if (!State.Assembly)
        {
                printf("Could not open file %s\n", argv[1]);
                return 1;
        }

        Instruction.Assemble = AssembleFoo;
        Instruction.Name = (STRING) "Foo";
        Instruction.Description = (STRING) "Testing";
        Instruction.Mnemonic = (STRING) "FOO";
        Arch.InstructionCount = 1;
        Arch.Instructions = &Instruction;
        Arch.Name = (STRING) "FOO Machine";
        Arch.Desc = (STRING) "Desc";
        Arch.Version = 1.0;

        State.Arch = &Arch;

        /* Main tokenising loop */
        State.CurrentToken = GetToken(&State);
        State.Expressions = ParseStatements(&State);
        Expr = State.Expressions;
        //DisplayExpressionTree(Expr, 0);
        GenerateForExpr(Expr);

        /* Cleanup */
        DeleteLabels(&State);
        fclose(State.Assembly);
        return (0);
}
