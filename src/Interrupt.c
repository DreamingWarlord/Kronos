#include "Interrupt.h"
#include "Processor.h"
#include "Bus.h"


static uint64 int_lines[64] = { 0 };
static uint64 int_mask = 0;
static uint64 int_targeted_lines = 0;


static void IntCntlReq(struct Region *reg, void *ptr, uint64 addr, uint64 size, bool write)
{
	if(size != 8 || (write && addr == INTCNTL_INT)) {
		Log(LOG_INT, "Invalid %s access %ul:%ul", write ? "write" : "read", addr, addr + size);
		return;
	}

	uint64 lines = 0;

	for(uint64 i = 0; i < 64; i++)
		lines |= (!!int_lines[i]) << i;

	switch(addr)
	{
	case INTCNTL_INT:
		memcpy(ptr, &lines, 8);
		return;
	case INTCNTL_MSK:
		if(write)
			memcpy(&int_mask, ptr, 8);
		else
			memcpy(ptr, &int_mask, 8);

		return;
	}
}

static struct Region int_reg = (struct Region) {
	INTCNTL_BASE,
	0x10,
	IntCntlReq
};

static void IntCntlUpdate(double delta, double time)
{
	for(uint64 i = 0; i < 64; i++) {
		if(((int_mask >> i) & 1) || (int_lines[i] == 0))
			continue;

		for(uint64 j = 0; j < ProcCount(); j++) {
			struct Processor *proc = ProcList()[j];

			if((proc->regs[REG_SCR] & SCR_IE) && !(proc->regs[REG_ICR] & (1ULL << (32 + i))))
				proc->int_req = TRUE;
		}
	}

	for(uint64 i = 0; i < 64; i++) {
		if(!((int_targeted_lines >> i) & 1))
			continue;

		struct Processor *proc = ProcList()[i];

		if(proc->regs[REG_SCR] & SCR_IE)
			proc->int_req = TRUE;
	}
}

void IntCntlInit()
{
	BusRegionAdd(&int_reg);
	LoopRegister(IntCntlUpdate);
}

void IntCntlAssert(uint8 line)
{
	Assert(line < 64);
	int_lines[line]++;
}

void IntCntlDeassert(uint8 line)
{
	Assert(line < 64);
	int_lines[line]--;
}

void IntCntlTargeted(uint32 proc)
{
	Assert(proc < ProcCount());
	int_targeted_lines |= 1ULL << proc;
}

void IntCntlTargetedEOI(uint32 proc)
{
	Assert(proc < ProcCount());
	int_targeted_lines &= ~(1ULL << proc);
}
