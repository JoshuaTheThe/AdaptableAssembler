#include"state.h"
#include"label.h"

BOOL ValidateLabels(AASState *State)
{
	LABEL *Label = NULL;
	_assert(State && State->CheckSum == STATE_CHECKSUM);
	if (!State)
	{
		return(FALSE);
	}
	Label = State->LabelsHead;
	if (!Label && State->LabelCount == (SIZE)0U)
	{
		return(TRUE); /* Empty */
	}

	while (Label)
	{
		if (Label->CheckSum != LABEL_CHECKSUM)
		{
			return(FALSE);
		}
		Label=Label->Next;
	}
	return(TRUE);
}

BOOL ValidateState(AASState *State)
{
	_assert(State && State->CheckSum == STATE_CHECKSUM);
	if (State && State->CheckSum == STATE_CHECKSUM && ValidateLabels(State))
	{
		return(TRUE);
	}

	printf("Invalid State\n  %p\n  Checksum = %zx, expected %zx\n  valid labels: %s\n",
			(void*)State, State->CheckSum, STATE_CHECKSUM, ValidateLabels(State) ? "TRUE" : "FALSE");
	return(FALSE);
}

AASState NewState(void)
{
	AASState State = {0};
	memset(&State, 0, sizeof(State));
	State.CheckSum = STATE_CHECKSUM;
	return(State);
}

BOOL DeleteLabels(AASState *State)
{
	LABEL *Label, *Next;
	if (!ValidateState(State))
	{
		return(FALSE);
	}

	Label = State->LabelsHead;
	while (Label)
	{
		Next = Label->Next;
		_assert(Label->Name);
		free(Label->Name);
		Label->Name = NULL;
		Label->CheckSum = 0;
		free(Label);
		Label = Next;
	}

	State->LabelCount = 0UL;
	State->LabelsHead = NULL;
	State->LabelsTail = NULL;
	return(TRUE);
}

