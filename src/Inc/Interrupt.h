#pragma once

#include "Common.h"


#define INTCNTL_BASE 0x100000000
#define INTCNTL_INT  0x00 // Interrupt lines
#define INTCNTL_MSK  0x08 // Mask register


void IntCntlInit();

void IntCntlAssert(uint8 line);

void IntCntlTargeted(uint32 proc);

void IntCntlTargetedEOI(uint32 proc);

void IntCntlDeassert(uint8 line);
