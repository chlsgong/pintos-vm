#include "vm/swap.h"
#include <debug.h>
#include <stdio.h>
#include "devices/block.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"

void swap_init() {
	int i;
	swap_device = block_get_role(BLOCK_SWAP);
	lock_init(&swap_lock);
	
	if (swap_device == NULL)
    	PANIC("No swap device found, can't initialize swap.");
    for(i = 0; i < NUM_SECTORS; i++) {
		swap_table[i].start = i * SLOT_SIZE;
		swap_table[i].free = 1;
		swap_table[i].owner = NULL;
		swap_table[i].upage = NULL;
    }
}

int swap_add(void* upage, void* kpage) {
    int i, j;
    int ofs = 0;

    lock_acquire(&swap_lock);
    for(i = 0; i < NUM_SECTORS; i++) {
		if(swap_table[i].free) { // found free swap slot
			pagedir_clear_page(thread_current()->pagedir, upage);
			swap_table[i].owner = thread_current();
			swap_table[i].upage = upage;
			swap_table[i].free = 0;
			for(j = 0; j < SLOT_SIZE; j++) { // write into swap slot
				block_write(swap_device, swap_table[i].start + j, kpage + ofs);
				ofs += BLOCK_SECTOR_SIZE;
			}
			lock_release(&swap_lock);
			return i;
		}
	}
	lock_release(&swap_lock);
	PANIC("No free swap slot available.");
}

int swap_remove(int slot, void* kpage) {
	int i, j;
	int ofs = 0;

    lock_acquire(&swap_lock);
	if(slot < 0 || swap_table[slot].free) {
	  	lock_release(&swap_lock);
	  	return 0;
	}
	swap_table[slot].free = 1;
	swap_table[slot].owner = NULL;
	swap_table[slot].upage = NULL;
	for(j = 0; j < SLOT_SIZE; j++) {
		block_read(swap_device, swap_table[slot].start + j, kpage + ofs);
		ofs += BLOCK_SECTOR_SIZE;
	}
	// zero out swap slot?
	lock_release(&swap_lock);
	return 1;
}

void swap_remove_process() {
	int i = 0;

    lock_acquire(&swap_lock);
	for(i = 0; i < NUM_SECTORS; i++) {
		if(swap_table[i].owner == thread_current()) {
			swap_table[i].free = 1;
			swap_table[i].owner = NULL;
			swap_table[i].upage = NULL;
		}
	}
	lock_release(&swap_lock);
}
