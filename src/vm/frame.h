#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/malloc.h"



struct frame {
	void* upage; //pointer to page that currently occupies this frame (user virtual address????)
	void* kpage; //pointer to this frame???? (kernel virtual address?????)
	struct thread* owner; //the thread that is using this frame
};

void frame_init(void);
void* frame_get_free(void);
void frame_release(void*);

#endif /* vm/frame.h */
