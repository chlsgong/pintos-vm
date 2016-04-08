#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/malloc.h"

struct frame* frames; //frame table
int length;

struct frame {
	void* upage; //pointer to page that currently occupies this frame (user virtual address????)
	void* kpage; //pointer to this frame???? (kernel virtual address?????)
	struct thread* owner; //the thread that is using this frame
	int occupied;
};

void frame_init(size_t);
void* frame_alloc(void);
int frame_set(void*, void*);
void frame_evict(void);
void frame_dealloc_all(void);
void frame_dealloc(void*);

#endif /* vm/frame.h */
