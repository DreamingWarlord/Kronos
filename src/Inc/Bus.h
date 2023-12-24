#pragma once

#include "Common.h"


struct Region
{
	uint64    base;
	uint64    size;
	void (*request) (struct Region *reg, void *ptr, uint64 addr, uint64 size, bool write);
};


void BusRegionAdd(struct Region *reg);

void BusReq(void *ptr, uint64 addr, uint64 size, bool write);
