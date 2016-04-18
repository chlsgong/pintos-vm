#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "filesys/file.h"
#include "filesys/filesys.h"
#include <list.h>
#include "devices/block.h"

/*Jorge Drove Here*/

/* Supplementary page table entry. */
struct sup_pte {
	void* page; // page that needs to be loaded
	struct file *file; // file that contains the data/program
	uint32_t page_read_bytes; // number of bytes read to a page
	uint32_t page_zero_bytes; // number of bytes left over that zeroes out a page
	bool writable; // writable flag for the file
	off_t offset; // file pointer offset
	struct list_elem sup_elem; // element for the supplementary page table list
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
