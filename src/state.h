#ifndef STATE_H
#define STATE_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"types.h"

/**
 * Return whether the state is valid.
 * */
BOOL ValidateState(AASState *State);
BOOL ValidateLabels(AASState *State);
AASState NewState(void);
BOOL DeleteLabels(AASState *State);

#endif

