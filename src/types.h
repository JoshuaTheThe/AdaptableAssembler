#ifndef TYPES_H
#define TYPES_H

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<unistd.h>
#include<assert.h>

typedef size_t SIZE;
typedef char *STRING;
typedef _Bool BOOL;
typedef double REAL;

#define FALSE ((BOOL)0)
#define TRUE ((BOOL)1)

#define STATE_CHECKSUM ((SIZE)0xBADFBADF)
#define LABEL_CHECKSUM ((SIZE)0xBEEFBAD0)

typedef struct
{
	STRING Name;
	STRING Description;
	void (*Assemble)(void);
} INSTRUCTION;

typedef struct
{
	STRING Name;
	STRING Desc;
	REAL   Version;
	INSTRUCTION *Instructions;
} ARCHITECTURE;

typedef enum
{
	SEGMENT_TEXT,
	SEGMENT_DATA,
} SEGMENT;

typedef struct LABEL
{
	/**
	 * Redundant Safety check.
	 * I trust no one */
	struct LABEL *Next;
	struct LABEL *Prev;
	STRING  Name;
	SIZE    CheckSum;
	SIZE    Offset;
	SEGMENT Segment;
	int     Padd;
} LABEL;

typedef struct
{
	/**
	 * Assume Worst case */
	LABEL *LabelsHead;
	LABEL *LabelsTail;
	SIZE   CheckSum;
	SIZE   LabelCount;
	FILE  *Assembly;
	FILE  *Output;
	ARCHITECTURE *Arch;
} AASState;

#ifndef NDEBUG
#define _assert(x) assert(x)
#else
#define _assert(x) if (!x) { printf("Assert Failed %s %s\n", __func__, __FILE__);  }
#endif

#endif

