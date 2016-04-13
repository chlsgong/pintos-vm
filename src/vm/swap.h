#ifndef VM_SWAP_H
#define VM_SWAP_H

#define NUM_SECTORS 1024
#define SLOT_SIZE 8

#include "devices/block.h"

struct swap_slot
{
	block_sector_t start;
	int free;
	struct thread* owner;
	void* upage;
};

struct swap_slot swap_table[NUM_SECTORS];
struct block* swap_device;

void swap_init(void);
int swap_add(void*, void*);
int swap_remove(int, void*);
void swap_remove_process(void);

#endif /* vm/swap.h */
