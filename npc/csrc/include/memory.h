#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"


void init_mem();
word_t paddr_read(paddr_t addr, int len);
void paddr_write(paddr_t addr, int len, word_t data);
word_t vaddr_ifetch(vaddr_t addr, int len);
word_t vaddr_read(vaddr_t addr, int len);
void vaddr_write(vaddr_t addr, int len, word_t data);

uint8_t* guest_to_host(paddr_t paddr);
paddr_t host_to_guest(uint8_t *haddr);

#endif
