#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#include"types.h"
#include"label.h"
#include"state.h"

int main(void)
{
	AASState State = NewState();
	LABEL *Lab = CreateLabel(&State);
	DeleteLabels(&State);
	Lab = (LABEL *)(NULL);
	(void)Lab;
	return(0);
}

