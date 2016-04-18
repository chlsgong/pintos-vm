/*Jorge Drove Here*/

#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "vm/page.h"
#include "userprog/pagedir.h"

struct frame* frames; // frame table
int length; // size of the frame table
int clock_ptr; // pointer used for the clock algorithm
struct lock frame_lock; // lock used for synchronizing frame functions
struct lock evict_lock; // lock used for synchronizing eviction procedures

/* Frame struct. */
struct frame {
	void* upage; // pointer to page that currently occupies this frame
	void* kpage; // pointer to this frame
	struct thread* owner; // the thread that is using this frame
	int occupied; // occupied flag
	uint32_t* pd; // process page directory
};

void frame_init(size_t);
void* frame_alloc(void*);
int frame_set(void*, void*);
void frame_dealloc_all(void);
void frame_dealloc(void*);
struct frame* frame_get_frame(void*);
void* frame_evict(void);

#endif /* vm/frame.h */
