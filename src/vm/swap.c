#include "vm/swap.h"
#include <debug.h>
#include <stdio.h>
#include "devices/block.h"
#include "threads/thread.h"
#include "vm/frame.h"
#include "userprog/pagedir.h"
#include "vm/page.h"

// Initializes the swap table and swap device
void swap_init() {
	/*Jorge Drove Here*/
	int i;
	swap_device = block_get_role(BLOCK_SWAP);
	
	if (swap_device == NULL)
    	PANIC("No swap device found, can't initialize swap.");
    for(i = 0; i < NUM_SECTORS; i++) {
		swap_table[i].start = i * SLOT_SIZE;
		swap_table[i].free = 1;
		swap_table[i].owner = NULL;
		swap_table[i].upage = NULL;
    }
}

// Adds a page to a swap slot and updates the swap table
int swap_add(void* kpage, void* upage, struct thread* t) {
	/*Charles Drove Here*/
    int i, j;
    int ofs = 0;

    lock_acquire(&frame_lock);
    for(i = 0; i < NUM_SECTORS; i++) {
		if(swap_table[i].free) { // found free swap slot
			swap_table[i].owner = t;
			swap_table[i].upage = upage;
			swap_table[i].free = 0;
			for(j = 0; j < SLOT_SIZE; j++) { // write into swap slot
				block_write(swap_device, swap_table[i].start + j, kpage + ofs);
				ofs += BLOCK_SECTOR_SIZE;
			}
			lock_release(&frame_lock);
			return i;
		}
	}
	lock_release(&frame_lock);
	PANIC("No free swap slot available.");
}

// Removes a page from swap and reads it to memory 
// updates the swap table
// Returns 1 if successful, 0 otherwise
int swap_remove(void* kpage, void* upage) {
	/*Jasmine Drove Here*/
	int i, j;
	int ofs = 0;

    lock_acquire(&frame_lock);
    for(i = 0; i < NUM_SECTORS; i++) {
		if((swap_table[i].owner == thread_current()) && (upage == swap_table[i].upage)) {
			swap_table[i].free = 1;
			swap_table[i].owner = NULL;
			swap_table[i].upage = NULL;
			for(j = 0; j < SLOT_SIZE; j++) {
				block_read(swap_device, swap_table[i].start + j, kpage + ofs);
				ofs += BLOCK_SECTOR_SIZE;
			}
			lock_release(&frame_lock);
			return 1;
		}
	}
	lock_release(&frame_lock);
	return 0;
}

// Removes the pages that belong to an exiting process from swap
void swap_remove_process() {
	/*Rebecca Drove Here*/
	int i = 0;	
    lock_acquire(&frame_lock);
	for(i = 0; i < NUM_SECTORS; i++) {
		if(swap_table[i].owner == thread_current()) {
			swap_table[i].free = 1;
			swap_table[i].owner = NULL;
			swap_table[i].upage = NULL;
		}
	}
	lock_release(&frame_lock);
}
