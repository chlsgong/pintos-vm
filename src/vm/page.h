#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "filesys/file.h"
#include "filesys/filesys.h"
#include <list.h>

struct sup_pte {
	void* swap_entry;
	void* frame;
	void* page;
	struct file *file; 
	uint32_t page_read_bytes;
	uint32_t page_zero_bytes;
	bool writable;
	off_t offset;
	struct list_elem sup_elem;
};

void page_add(struct sup_pte*, void*, struct file*,
			 uint32_t, uint32_t, bool, off_t);

struct sup_pte *page_get(void*);
void page_destroy(void*);

#endif /* vm/page.h */
