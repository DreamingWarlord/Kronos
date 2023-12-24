#pragma once

#include "Common.h"


#define REG_IP   0
#define REG_SP   1
#define REG_UR   2
#define REG_KR   3
#define REG_IHP  4
#define REG_IRA  5
#define REG_IRC  6
#define REG_AVA  7
#define REG_ECR  8
#define REG_TMA  9
#define REG_TRA 10
#define REG_SCR 11
#define REG_ICR 12
#define REG_IPI 13

#define EV_NOEXC 0
#define EV_INVOP 1
#define EV_ACVIO 2
#define EV_ALIGN 3

#define ECR_EV 3
#define ECR_WR 4

#define SCR_ASID 511
#define SCR_IE   (1ULL << 9)
#define SCR_PV   (1ULL << 10)
#define SCR_MMU  (1ULL << 11)
#define SCR_HLT  (1ULL << 12)

#define ICR_CORE 0xFFFFFFFF
#define ICR_MASK 0xFFFFFFFF00000000

#define IPI_DEST 0xFFFFFFFF
#define IPI_EXCL (1ULL << 32)

#define TLB_ASID 511
#define TLB_PR   (1ULL << 9)
#define TLB_WR   (1ULL << 10)
#define TLB_CE   (1ULL << 11)
#define TLB_ADDR (~4095ULL)


struct Processor
{
	pthread_t     thread;
	pthread_mutex_t lock;
	bool         int_req;
	uint32            id;
	uint64        cycles;
	uint64          regs[32];
};


bool ProcReq(struct Processor *proc, void *ptr, uint64 addr, uint64 size, bool write);

void ProcCycle(struct Processor *proc);

void ProcEntry(struct Processor *proc);

struct Processor **ProcList();

uint64 ProcCount();

void ProcInit(uint64 count);
