#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state.h"
#include "label.h"

LABEL *InsertLabel(LABEL *Label, ArborState *State)
{
        _assert(ValidateState(State));
        _assert(Label && Label->CheckSum == LABEL_CHECKSUM);
        if (!Label || !ValidateState(State))
        {
                return (LABEL *)(NULL);
        }

        if (State->LabelCount == 0 || !State->LabelsHead || !State->LabelsTail)
        {
                _assert(State->LabelCount == 0 &&
                        State->LabelsHead == NULL &&
                        State->LabelsTail == NULL);
                State->LabelsHead = Label;
                State->LabelsTail = Label;
                State->LabelCount += 1;
        }
        else if (State->LabelCount > 0 || State->LabelsHead || State->LabelsTail)
        {
                _assert(State->LabelCount > 0 &&
                        State->LabelsHead &&
                        State->LabelsTail);
                Label->Next = State->LabelsTail->Next;
                Label->Prev = State->LabelsTail;
                State->LabelsTail->Next = Label;
                State->LabelsTail = Label;
                State->LabelCount += 1;
        }
        else if (Label)
        {
                printf("Could Not create label @%p, deleting.\n", (void *)Label);
                free(Label);
                return (LABEL *)(NULL);
        }

        return (Label);
}

LABEL *CreateLabel(ArborState *State)
{
        LABEL *NewLabel = (LABEL *)calloc(1, sizeof(*NewLabel));
        _assert(State);
        if (ValidateState(State) || !NewLabel)
        {
                if (NewLabel)
                        free(NewLabel);
                return (LABEL *)(NULL);
        }

        NewLabel->CheckSum = LABEL_CHECKSUM;
        InsertLabel(NewLabel, State);
        return (NewLabel);
}
