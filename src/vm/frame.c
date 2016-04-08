#include "vm/frame.h"
#include "threads/loader.h"
#include <stdint.h>
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/malloc.h"

#include "threads/vaddr.h"
#include <stdio.h>

// static struct lock frame_lock;

// Initializes the frame table
void frame_init(size_t size) {
	unsigned i;
	struct frame f;
	frames = malloc(sizeof(struct frame) * size);
	length = 0;
	// lock_init(&frame_lock);	

	for(i = 0; i < size-1; i++) {
		f = frames[i];
		f.kpage = palloc_get_page(PAL_USER | PAL_ZERO);
		f.upage = NULL;
		f.owner = NULL;
		frames[i] = f;
		length++;
	}
}


// Allocates a frame given a user address
void* frame_alloc() {
	int i = 0;
	for(i = 0; i < length; i++) {
		if(frames[i].owner == NULL) {
			frames[i].owner = thread_current();
			return frames[i].kpage;
		}
	}
	// printf("\n\nkpage val after loop: %p, thread: %s\n\n", kpage, thread_current()->name);
	return NULL;
}

// Sets a frame's mapping to a different user address
// Useful for eviction
int frame_set(void* upage, void* kpage) {
	int i;
	int found = 0;
	for(i = 0; i < length; i++) {
		if(frames[i].kpage == kpage) {
			frames[i].upage = upage;
			found = 1;
			break;
		}
	}
	return found;
}

// void frame_evict () {
// 	for(i = 0; i < length; i++) {
// 		if(frames[i]->owner == NULL) {
// 			frames[i]->owner = thread_current();
// 			kpage = frames[i]->kpage;
// 			full = 0;
// 			break;
// 		}
// 	}
// }


// Deallocates and frees a frame
void frame_dealloc(void* kpage) {
	int i;
	for(i = 0; i < length; i++) {
		if(frames[i].kpage == kpage) {
			frames[i].owner = NULL;
			frames[i].upage = NULL;
			frames[i].kpage = NULL;
			break;
		}
	}
}