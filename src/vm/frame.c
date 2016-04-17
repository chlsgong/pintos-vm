#include "vm/frame.h"
#include "threads/loader.h"
#include <stdint.h>
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/malloc.h"
#include "vm/page.h"
#include "userprog/pagedir.h"

#include "threads/vaddr.h"
#include <stdio.h>

 //static struct lock frame_lock;

// Initializes the frame table
void frame_init(size_t size) {
	unsigned i;
	struct frame f;
	frames = malloc(sizeof(struct frame) * size);
	length = 0;
	lock_init(&frame_lock);	
	lock_init(&evict_lock);	

	for(i = 0; i < size-1; i++) {
		f = frames[i];
		f.kpage = palloc_get_page(PAL_USER | PAL_ZERO);
		f.upage = NULL;
		f.owner = NULL;
		f.occupied = 0;
		f.evictable = 1;
		f.pte = NULL;
		f.pd = NULL;
		frames[i] = f;
		length++;
	}
	clock_ptr = 0;
}


// Allocates a frame given a user address
void* frame_alloc(void* upage) {
	lock_acquire(&frame_lock);
	int i = 0;
	void* kpage = NULL;
	for(i = 0; i < length; i++) {
		if(!frames[i].occupied) {
			frames[i].owner = thread_current();
			frames[i].pd = thread_current()->pagedir;
			frames[i].upage = upage;
			frames[i].occupied = 1;
			kpage = frames[i].kpage;
			break;
		}
	}
	lock_release(&frame_lock);
	return kpage;
}



// Sets a frame's mapping to a different user address
// Useful for eviction
int frame_set(void* upage, void* kpage) {
	lock_acquire(&frame_lock);
	int i;
	int found = 0;
	for(i = 0; i < length; i++) {
		if(frames[i].kpage == kpage) {
			frames[i].upage = upage;
			frames[i].owner = thread_current();
			frames[i].pd = thread_current()->pagedir;
			frames[i].occupied = 1;
			found = 1;
			break;
		}
	}
	lock_release(&frame_lock);
	return found;
}




void* frame_get_upage(void* kpage) {
	lock_acquire(&frame_lock);
	int i;
	void* upage = NULL;
	for(i = 0; i < length; i++) {
		if(frames[i].kpage == kpage) {
			upage = frames[i].upage;
			break;
		}
	}
	lock_release(&frame_lock);
	return upage;
}


void frame_dealloc_all() {
	lock_acquire(&frame_lock);
	int i;
	for(i = 0; i < length; i++) {
		if(frames[i].owner == thread_current()) {
			pagedir_clear_page(thread_current()->pagedir, frames[i].upage);
			frames[i].owner = NULL;
			frames[i].upage = NULL;
			frames[i].pd = NULL;
			frames[i].occupied = 0;
		}
	}
    lock_release(&frame_lock);
}


// Deallocates and frees a frame
void frame_dealloc(void* kpage, uint32_t* owner_pd, void* upage) {
	lock_acquire(&frame_lock);
	int i;
	for(i = 0; i < length; i++) {
		if(frames[i].kpage == kpage) {
			if(upage != NULL)
				pagedir_clear_page(owner_pd, upage);
			else {
				printf("UPAGE NULL DEALLOC :(\n");
				exit(-1);
			}
			frames[i].owner = NULL;
			frames[i].upage = NULL;
			frames[i].pd = NULL;
			frames[i].occupied = 0;
			break;
		}
	}
	lock_release(&frame_lock);
}



void* frame_evict() {
  lock_acquire(&frame_lock);
  void* freed_frame = NULL;
  void* upage = NULL;
  uint32_t* pd = NULL;
  while(!freed_frame) {
    upage = frames[clock_ptr].upage;
    pd = frames[clock_ptr].pd;
    if(page_is_accessed(pd, upage)) {
      page_set_accessed(pd, upage, false);
    } else {
      freed_frame = frames[clock_ptr].kpage;
    }
    if(clock_ptr == (length - 1)) {
      clock_ptr = 0;
    } else {
      clock_ptr++;
    }
  }
  lock_release(&frame_lock);
  return freed_frame;
}


struct frame* frame_get_frame(void* kpage) {
	lock_acquire(&frame_lock);
	int i;
	for(i = 0; i < length; i++) {
		if(frames[i].kpage == kpage) {
			lock_release(&frame_lock);
			return &frames[i];
		}
	}
	lock_release(&frame_lock);
	return NULL;
}
