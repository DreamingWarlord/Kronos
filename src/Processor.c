#include "Processor.h"
#include "OpCodes.h"
#include "Bus.h"


#define MemReq(ptr, addr, size, write) if(!ProcReq(proc, ptr, addr, size, write)) return
#define PrivCheck() if(proc->regs[REG_SCR] & SCR_PV) { proc->regs[REG_ECR] = EV_INVOP; proc->int_req = TRUE; return; }


static struct Processor *processors[PROC_MAX] = { NULL };

static uint64 processorc = 0;


struct Processor **ProcList()
{
	return processors;
}

uint64 ProcCount()
{
	return processorc;
}

void ProcInit(uint64 count)
{
	for(uint64 i = 0; i < count; i++) {
		struct Processor *proc = calloc(1, sizeof(struct Processor));
		proc->id = processorc;
		proc->regs[REG_ICR] = proc->id | ICR_MASK;

		if(proc->id != 0) {
			proc->regs[REG_IHP] = 0x1FF0000;
			proc->regs[REG_SCR] |= SCR_HLT | SCR_IE;
		}

		processors[processorc++] = proc;
		pthread_mutex_init(&proc->lock, NULL);
		pthread_create(&proc->thread, NULL, (void *(*)(void *)) ProcEntry, proc);
	}
}

bool ProcReq(struct Processor *proc, void *ptr, uint64 addr, uint64 size, bool write)
{
	if(addr & (size - 1)) {
		proc->regs[REG_ECR] = EV_ALIGN | (write ? ECR_WR : 0);
		proc->int_req = TRUE;
		return FALSE;
	}

	if(proc->regs[REG_SCR] & SCR_MMU) {
		// TODO: Handle MMU
	}

	BusReq(ptr, addr, size, write);
	return TRUE;
}

void ProcCycle(struct Processor *proc)
{
	proc->cycles++;

	if(proc->int_req) {
		proc->regs[REG_IRA] = proc->regs[REG_IP];
		proc->regs[REG_ICR] = !!(proc->regs[REG_SCR] & SCR_PV) | (!!(proc->regs[REG_SCR] & SCR_IE) << 1);
		proc->regs[REG_SCR] &= ~(SCR_IE | SCR_PV);
		proc->regs[REG_IP] = proc->regs[REG_IHP];
		proc->int_req = FALSE;
	}

	uint8 opcode = 0;
	uint8 operands = 0;
	uint8 instr[8] = { 0 };
	uint64 imm = 0;
	uint64 regs[32];
	memcpy(regs, proc->regs, 256);

	switch(opcode)
	{
	case OPC_JMP:
	case OPC_CALL:
		MemReq(instr, regs[REG_IP], 4, FALSE);
		regs[REG_IP] += 4;
		memcpy(&imm, &instr[1], 3);
		imm = (int64) (((int32) (imm << 8)) >> 8);
		break;
	case OPC_JC:
		MemReq(instr, regs[REG_IP], 4, FALSE);
		regs[REG_IP] += 4;
		memcpy(&imm, &instr[2], 2);
		imm = (int64) (int16) imm;
		break;
	case OPC_IMM16:
		MemReq(instr, regs[REG_IP], 4, FALSE);
		regs[REG_IP] += 4;
		memcpy(&imm, &instr[2], 2);
	case OPC_IMM48:
		MemReq(instr, regs[REG_IP], 8, FALSE);
		regs[REG_IP] += 8;
		memcpy(&imm, &instr[2], 6);
	default:
		if(opcode > 7) {
			MemReq(instr, regs[REG_IP], 2, FALSE);
			regs[REG_IP] += 2;
		} else {
			MemReq(instr, regs[REG_IP]++, 1, FALSE);
		}

		break;
	}

	opcode = instr[0];
	operands = instr[1];

	switch(opcode)
	{
	case OPC_NOP:
		break;
	case OPC_WBC:
		PrivCheck();
		break;
	case OPC_INVPG:
		PrivCheck();
		break;
	case OPC_RET:
		regs[REG_SP] -= 8;
		MemReq(&regs[REG_IP], regs[REG_SP], 8, FALSE);
		break;
	case OPC_TLBRET:
		PrivCheck();
		break;
	case OPC_IRET:
		PrivCheck();
		regs[REG_IP] = regs[REG_IRA];
		regs[REG_SCR] |= (regs[REG_ICR] & 1) ? SCR_PV : 0;
		regs[REG_SCR] |= (regs[REG_ICR] & 2) ? SCR_IE : 0;
		break;
	case OPC_UD:
		proc->regs[REG_ECR] = EV_INVOP;
		proc->int_req = TRUE;
		return;
	case OPC_FENCE:
		break;
	case OPC_PUSH:
		MemReq(&regs[operands & 0xF], regs[REG_SP], 8, TRUE);
		regs[REG_SP] += 8;
		break;
	case OPC_POP:
		regs[REG_SP] -= 8;
		MemReq(&regs[operands & 0xF], regs[REG_SP], 8, FALSE);
		break;
	case OPC_NEG:
		regs[operands & 0xF] = -regs[operands & 0xF];
		break;
	case OPC_NOT:
		regs[operands & 0xF] = ~regs[operands & 0xF];
		break;
	case OPC_EXT8:
		regs[operands & 0xF] = (int64) (int8) regs[operands & 0xF];
		break;
	case OPC_EXT16:
		regs[operands & 0xF] = (int64) (int16) regs[operands & 0xF];
		break;
	case OPC_EXT32:
		regs[operands & 0xF] = (int64) (int32) regs[operands & 0xF];
		break;
	case OPC_JMPR:
		regs[REG_IP] = regs[operands & 0xF];
		break;
	case OPC_CALLR:
		MemReq(&regs[REG_IP], regs[REG_SP], 8, TRUE);
		regs[REG_SP] += 8;
		regs[REG_IP] = regs[operands & 0xF];
		break;
	case OPC_TLBFILL:
		PrivCheck();
		break;
	case OPC_JMP:
		regs[REG_IP] += imm - 4;
		break;
	case OPC_CALL:
		MemReq(&regs[REG_IP], regs[REG_SP], 8, TRUE);
		regs[REG_SP] += 8;
		regs[REG_IP] += imm - 4;
		break;
	case OPC_IMM16:
	case OPC_IMM48:
		regs[operands & 0xF] = imm;
		break;
	case OPC_JC:
		if(!(regs[operands & 0xF] & regs[operands >> 4]))
			regs[REG_IP] += imm - 4;

		break;
	case OPC_MOV:
		regs[operands & 0xF] = regs[operands >> 4];
		break;
	case OPC_LDSP:
		if((operands >> 4) > 1 && (regs[REG_SCR] & SCR_PV))
			PrivCheck();

		regs[operands & 0xF] = regs[(operands >> 4) + REG_SP];
		break;
	case OPC_STSP:
		if((operands >> 4) != 0 && (regs[REG_SCR] & SCR_PV))
			PrivCheck();

		regs[(operands & 0xF) + REG_SP] = regs[operands >> 4];
		break;
	case OPC_LOAD8:
		MemReq(&regs[operands & 0xF], regs[operands >> 4], 1, FALSE);
		break;
	case OPC_LOAD16:
		MemReq(&regs[operands & 0xF], regs[operands >> 4], 2, FALSE);
		break;
	case OPC_LOAD32:
		MemReq(&regs[operands & 0xF], regs[operands >> 4], 4, FALSE);
		break;
	case OPC_LOAD64:
		MemReq(&regs[operands & 0xF], regs[operands >> 4], 8, FALSE);
		break;
	case OPC_STORE8:
		MemReq(&regs[operands >> 4], regs[operands & 0xF], 1, TRUE);
		break;
	case OPC_STORE16:
		MemReq(&regs[operands >> 4], regs[operands & 0xF], 2, TRUE);
		break;
	case OPC_STORE32:
		MemReq(&regs[operands >> 4], regs[operands & 0xF], 4, TRUE);
		break;
	case OPC_STORE64:
		MemReq(&regs[operands >> 4], regs[operands & 0xF], 8, TRUE);
		break;
	case OPC_ADD:
		regs[operands & 0xF] += regs[operands >> 4];
		break;
	case OPC_SUB:
		regs[operands & 0xF] -= regs[operands >> 4];
		break;
	case OPC_MUL:
		regs[operands & 0xF] *= regs[operands >> 4];
		break;
	case OPC_DIV:
		regs[operands & 0xF] /= regs[operands >> 4];
		break;
	case OPC_REM:
		regs[operands & 0xF] %= regs[operands >> 4];
		break;
	case OPC_AND:
		regs[operands & 0xF] &= regs[operands >> 4];
		break;
	case OPC_OR:
		regs[operands & 0xF] |= regs[operands >> 4];
		break;
	case OPC_XOR:
		regs[operands & 0xF] ^= regs[operands >> 4];
		break;
	case OPC_SHL:
		regs[operands & 0xF] <<= regs[operands >> 4] & 63;
		break;
	case OPC_SHR:
		regs[operands & 0xF] >>= regs[operands >> 4] & 63;
		break;
	case OPC_CMP:
		regs[operands & 0xF] = 0;

		if(regs[operands & 0xF] == regs[operands >> 4])
			regs[operands & 0xF] |= 1;

		if(regs[operands & 0xF] > regs[operands >> 4])
			regs[operands & 0xF] |= 2;

		if(regs[operands & 0xF] < regs[operands >> 4])
			regs[operands & 0xF] |= 4;

		break;
	case OPC_SCMP:
		regs[operands & 0xF] = 0;

		if((int64) regs[operands & 0xF] == (int64) regs[operands >> 4])
			regs[operands & 0xF] |= 1;

		if((int64) regs[operands & 0xF] > (int64) regs[operands >> 4])
			regs[operands & 0xF] |= 2;

		if((int64) regs[operands & 0xF] < (int64) regs[operands >> 4])
			regs[operands & 0xF] |= 4;

		break;
	}

	memcpy(proc->regs, regs, 256);
}

void ProcEntry(struct Processor *proc)
{
	struct timespec tp_now;
	struct timespec tp_last;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tp_now);
	tp_last = tp_now;

	while(TRUE) {
		if(!(proc->regs[REG_SCR] & SCR_HLT)) {
			pthread_mutex_lock(&proc->lock);
			ProcCycle(proc);
			pthread_mutex_unlock(&proc->lock);
		}

		clock_gettime(CLOCK_MONOTONIC_RAW, &tp_now);

		if((tp_now.tv_nsec - tp_last.tv_nsec) >= 12500000) {
			tp_last = tp_now;
			nanosleep(&(struct timespec) { 0, 25000000 }, NULL);
		}
	}
}
