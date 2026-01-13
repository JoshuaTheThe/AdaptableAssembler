#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "types.h"
#include "label.h"
#include "state.h"
#include "token.h"
#include "parser.h"
#include "codegen.h"

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

int main(int argc, char **argv)
{
        /* Checks and Init */
        ArborState State;
        /* Unused, in future, codegen will use custom arch */
        ARCHITECTURE Arch;
        INSTRUCTION Instruction;
        if (argc != 3)
        {
                printf("Invalid Usage\n%s <source> <output>\n", argv[0]);
                return 1;
        }

        State = NewState();
        State.Assembly = fopen(argv[1], "r");
        State.Output = fopen(argv[2], "w");
        if (!State.Assembly || !State.Output)
        {
                printf("Could not open file %s or %s\n", argv[1], argv[2]);
                if (State.Assembly) fclose(State.Assembly);
                if (State.Output) fclose(State.Output);
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
        EXPRESSION *AST = ParseStatements(&State);
        DisplayExpressionTree(AST, 0);
        EmitStart(&State);
        GenerateForExpr(&State, AST);

        /* Cleanup */
        DeleteLabels(&State);
        fclose(State.Assembly);
        fclose(State.Output);
        return (0);
}
