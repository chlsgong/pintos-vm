#ifndef VM_SWAP_H
#define VM_SWAP_H

#define NUM_SECTORS 1024
#define SLOT_SIZE 8

#include "devices/block.h"
#include "vm/page.h"

/*Jorge Drove Here*/

/*Swap entry.*/
struct swap_slot
{
	block_sector_t start; // start of the block that contains the data
	void* upage; // page that is in swap
	int free; // flag denoting if a slot is free
	struct thread* owner; // process that owns the upage
};

struct swap_slot swap_table[NUM_SECTORS]; // swap table
struct block* swap_device; // actual block device that the swap is located in

void swap_init(void);
int swap_add(void*, void*, struct thread*);
int swap_remove(void*, void*);
void swap_remove_process(void);

#endif /* vm/swap.h */
