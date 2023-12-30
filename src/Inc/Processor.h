#pragma once

#include "Common.h"


#define REG_IP  16
#define REG_SP  17
#define REG_UR  18
#define REG_KR  19
#define REG_IHP 20
#define REG_IRA 21
#define REG_IRC 22
#define REG_AVA 23
#define REG_ECR 24
#define REG_TMA 25
#define REG_TRA 26
#define REG_TRC 27
#define REG_THP 28
#define REG_SCR 29
#define REG_ICR 30
#define REG_IPI 31

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
	uint64      tlb_addr[64];
	uint64      tlb_data[64];
	uint64      tlb_hint;
};


bool ProcReq(struct Processor *proc, void *ptr, uint64 addr, uint64 size, bool write);

void ProcCycle(struct Processor *proc);

void ProcEntry(struct Processor *proc);

struct Processor **ProcList();

uint64 ProcCount();

void ProcInit(uint64 count);
