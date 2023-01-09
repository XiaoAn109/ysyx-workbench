#include "../include/memory.h"

#define MEM_SIZE 0x8000000

uint8_t *pmem = NULL;
uint8_t *inst_mem = pmem;
void init_mem() {
	pmem = (uint8_t*)malloc(MEM_SIZE); //128MB instruction memory
	assert(pmem);
	Log("physical memory area [0x%016x, 0x%016x]", BASE_ADDR, BASE_ADDR+MEM_SIZE-1);
  // FILE *fp = fopen(testcase, "r");
	// uint32_t *p = (uint32_t *)pmem;
	// while(fscanf(fp, "%x", p ++) != EOF);
  //   fclose(fp);
}

static inline word_t host_read(void *addr, int len) {
  switch (len) {
    case 1: return *(uint8_t  *)addr;
    case 2: return *(uint16_t *)addr;
    case 4: return *(uint32_t *)addr;
    case 8: return *(uint64_t *)addr;
    default: assert(0);
  }
}

static inline void host_write(void *addr, int len, word_t data) {
  switch (len) {
    case 1: *(uint8_t  *)addr = data; return;
    case 2: *(uint16_t *)addr = data; return;
    case 4: *(uint32_t *)addr = data; return;
    case 8: *(uint64_t *)addr = data; return;
    default: assert(0);
  }
}
uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - BASE_ADDR; }
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + BASE_ADDR; }

static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

word_t paddr_read(paddr_t addr, int len) {
  printf(ANSI_FG_GREEN "[Read  %d Bytes from addr: 0x%016lx]\n" ANSI_NONE, len, addr);
  return pmem_read(addr, len);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {
  printf(ANSI_FG_GREEN "[Write %d Bytes data 0x%016lx to   addr: 0x%016lx]\n" ANSI_NONE, len, data, addr);
  pmem_write(addr, len, data); 
	return;
}

word_t vaddr_ifetch(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

word_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  paddr_write(addr, len, data);
}

extern "C" void rtl_pmem_read(long long raddr, long long *rdata, svBit ren) {
	// raddr = raddr & ~0x7ull;
	if(ren && raddr >= BASE_ADDR && raddr <= BASE_ADDR+MEM_SIZE-1) {
		*rdata = paddr_read(raddr, 8);
	}
	else
		*rdata = 0;
}

extern "C" void rtl_pmem_write(long long waddr, long long wdata, char wmask, svBit wen) {
	// waddr = waddr & ~0x7ull;
	if(wen) {
		switch((unsigned char)wmask) {
			case 1: paddr_write(waddr, 1, wdata); break;
			case 3: paddr_write(waddr, 2, wdata); break;
			case 15: paddr_write(waddr, 4, wdata); break;
			case 255: paddr_write(waddr, 8, wdata); break;
			default: break;
		}
	}
}
