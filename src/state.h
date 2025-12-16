#ifndef STATE_H
#define STATE_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"types.h"

/**
 * Return whether the state is valid.
 * */
BOOL ValidateState(ArborState *State);
BOOL ValidateLabels(ArborState *State);
ArborState NewState(void);
BOOL DeleteLabels(ArborState *State);

#endif

