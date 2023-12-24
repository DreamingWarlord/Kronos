#include "Bus.h"


static struct Region *bus_regions[256] = { NULL };
static uint64 bus_regionc = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


void BusRegionAdd(struct Region *reg)
{
	bus_regions[bus_regionc++] = reg;
}

void BusReq(void *ptr, uint64 addr, uint64 size, bool write)
{
	Assert((size & (size - 1)) == 0);
	Assert((addr & (size - 1)) == 0);
	pthread_mutex_lock(&lock);
	struct Region *reg = NULL;

	for(uint64 i = 0; i < bus_regionc; i++) {
		reg = bus_regions[i];

		if(addr >= reg->base && (addr + size) <= (reg->base + reg->size))
			break;

		reg = NULL;
	}

	if(reg == NULL)
		return;

	reg->request(reg, ptr, addr - reg->base, size, write);
	pthread_mutex_unlock(&lock);
}
