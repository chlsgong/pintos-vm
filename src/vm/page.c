#include "vm/page.h"
#include <list.h>
#include "threads/thread.h"

void page_add(struct sup_pte* entry, void* page, struct file *file, uint32_t page_read_bytes,
	uint32_t page_zero_bytes, bool writable, off_t offset) {
	
	entry->swap_entry = NULL;
	entry->page = page;
	entry->file = file; 
	entry->page_read_bytes = page_read_bytes;
	entry->page_zero_bytes = page_zero_bytes;
	entry->writable = writable;
	entry->offset = offset;
	list_push_back(&thread_current()->sup_page_table, &entry->sup_elem);
}

struct sup_pte* page_get(void* upage) {
	struct list_elem* e;
	struct sup_pte* pte;
	for (e = list_begin (&thread_current()->sup_page_table); 
		 e != list_end (&thread_current()->sup_page_table);
		 e = list_next (e))
	{
		pte = list_entry (e, struct sup_pte, sup_elem);
		if(upage == pte->page) {
			printf("upage: %p, page: %p\n", upage, pte->page);
			return pte;
		}
	}
	return NULL;
}

