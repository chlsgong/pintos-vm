#include "vm/frame.h"
#include "threads/loader.h"
#include <stdint.h>
#include "threads/thread.h"

static struct frame** frames; //frame table

void frame_init() {
	unsigned i;
	frames = malloc(sizeof(struct frame*) * init_ram_pages);
	for(i = 0; i < init_ram_pages; i++) {
		struct frame* f = malloc(sizeof(struct frame)); // double check that we need to do this
		f->kpage = palloc_get_page(PAL_USER);
		f->upage = NULL;
		f->owner = NULL;
		frames[i] = f;
	}
}


void* frame_get_free() {
	void* kpage;
	int i;
	int length = sizeof(frames) / sizeof(struct frame*);
	int full = 1;
	for(i = 0; i < length; i++) {
		if(frames[i]->owner == NULL) {
			frames[i]->owner = thread_current();
			kpage = frames[i]->kpage;
			full = 0;
			break;
		}
	}
	if(full)
		kpage = NULL;
	return kpage;
}


void frame_release(void* kpage) {
	int i;
	int length = sizeof(frames) / sizeof(struct frame*);
	for(i = 0; i < length; i++) {
		if(frames[i]->kpage == kpage) {
			frames[i]->owner = NULL;
			frames[i]->upage = NULL;
			palloc_free_page(kpage);
			break;
		}
	}
}

