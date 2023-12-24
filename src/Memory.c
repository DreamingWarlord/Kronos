#include "Memory.h"
#include "Bus.h"


static FILE *file_ram = NULL;

static FILE *file_rom = NULL;

static void RAMCntlReq(struct Region *reg, void *ptr, uint64 addr, uint64 size, bool write)
{
	fseek(file_ram, addr, SEEK_SET);

	if(write)
		fwrite(ptr, size, 1, file_ram);
	else
		fread(ptr, size, 1, file_ram);
}

static void ROMCntlReq(struct Region *reg, void *ptr, uint64 addr, uint64 size, bool write)
{
	if(write) {
		Log(LOG_MEM, "ROM: Write request ignored");
		return;
	}

	fseek(file_rom, addr, SEEK_SET);
	fread(ptr, size, 1, file_rom);
}

static struct Region reg_rom = (struct Region) {
	0,
	RAM_BASE,
	ROMCntlReq
};

static struct Region reg_ram = (struct Region) {
	RAM_BASE,
	0,
	RAMCntlReq
};


void RAMCntlInit(uint64 size)
{
	file_ram = tmpfile();
	reg_ram.size = size;
	BusRegionAdd(&reg_ram);
}

void ROMCntlInit(FILE *rom_file)
{
	file_rom = rom_file;
	BusRegionAdd(&reg_rom);
}
