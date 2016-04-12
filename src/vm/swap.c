#include "vm/swap.h"
#include <debug.h>
#include <stdio.h>
#include "devices/block.h"

//struct block* swap_device;

void swap_init() {
	swap_device = block_get_role(BLOCK_SWAP);
	printf("\nblock size: %d\n", block_size(swap_device));
	if (swap_device == NULL)
    	PANIC ("No swap device found, can't initialize swap.");
}

