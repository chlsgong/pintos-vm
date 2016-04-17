#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "filesys/file.h"
#include "filesys/filesys.h"
#include <list.h>
#include "devices/block.h"

struct sup_pte {
	int swap_entry;
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
void page_add_sp(struct sup_pte*, void*);
struct sup_pte *page_get(void*);
void page_set_swap(struct sup_pte*, int);
void page_destroy(void);
void page_set_accessed(uint32_t*, void*, bool);
bool page_is_accessed(uint32_t*, void*);

#endif /* vm/page.h */
