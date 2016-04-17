#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "vm/page.h"
#include "userprog/pagedir.h"

struct frame* frames; //frame table
int length;
int clock_ptr;
struct lock frame_lock;
struct lock evict_lock;

struct frame {
	void* upage; //pointer to page that currently occupies this frame (user virtual address????)
	void* kpage; //pointer to this frame???? (kernel virtual address?????)
	struct thread* owner; //the thread that is using this frame
	int occupied;
	int evictable;
	struct sup_pte* pte;
	uint32_t* pd;
};

void frame_init(size_t);
void* frame_alloc(void*);
// int frame_set(void*, void*, struct sup_pte*);
int frame_set(void*, void*);

void* frame_get_upage(void*);
//void frame_evict(void);
void frame_dealloc_all(void);
//void frame_dealloc(void*);

void frame_dealloc(void*, uint32_t*, void*);
struct frame* frame_get_frame(void*);

void* frame_evict(void);

#endif /* vm/frame.h */
