#ifndef VM_SWAP_H
#define VM_SWAP_H

#define NUM_SECTORS 1024
#define SLOT_SIZE 8

#include "devices/block.h"
#include "vm/page.h"

struct swap_slot
{
	block_sector_t start;
	void* upage;
	int free;
	struct thread* owner;
};

struct swap_slot swap_table[NUM_SECTORS];
struct block* swap_device;
struct lock swap_lock;

void swap_init(void);
// int swap_add(void*, void*, struct thread*);
int swap_add(void*, void*, struct thread*);

int swap_remove(void*, void*);
void swap_remove_process(void);

#endif /* vm/swap.h */
