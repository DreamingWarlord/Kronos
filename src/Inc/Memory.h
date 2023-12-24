#pragma once

#include "Common.h"


#define RAM_BASE 0x80000


void RAMCntlInit(uint64 size);

void ROMCntlInit(FILE *rom_file);
