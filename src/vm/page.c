#include "vm/page.h"
#include <list.h>
#include "threads/thread.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "threads/synch.h"

// Adds an entry to the supplementary page table 
void page_add(struct sup_pte* entry, void* page, struct file *file, uint32_t page_read_bytes,
	uint32_t page_zero_bytes, bool writable, off_t offset) {
	/*Charles Drove Here*/
	
	entry->page = page;
	entry->file = file; 
	entry->page_read_bytes = page_read_bytes;
	entry->page_zero_bytes = page_zero_bytes;
	entry->writable = writable;
	entry->offset = offset;
	list_push_back(&thread_current()->sup_page_table, &entry->sup_elem);
}

// Adds an entry to the supplementary page table for stack growth
void page_add_sp(struct sup_pte* entry, void* page) {
	/*Charles Drove Here*/
	entry->page = page;
	entry->writable = true;
	list_push_back(&thread_current()->sup_page_table, &entry->sup_elem);
}

// Returns a supplementary page table entry for a given upage
struct sup_pte* page_get(void* upage) {
	/*Jasmine Drove Here*/
	struct list_elem* e;
	struct sup_pte* pte;
	for (e = list_begin (&thread_current()->sup_page_table); 
		 e != list_end (&thread_current()->sup_page_table);
		 e = list_next (e))
	{
		pte = list_entry (e, struct sup_pte, sup_elem);
		if(upage == pte->page) {
			return pte;
		}
	}
	return NULL;
}

// Deallocates all the supplementary page table entries of the calling process
void page_destroy() {
	/*Jasmine Drove Here*/
	struct list_elem* e;
	struct sup_pte* pte;
	struct thread* t = thread_current();

	if(!list_empty(&t->sup_page_table)) {
		e = list_begin (&t->sup_page_table);
		while(e != list_end (&t->sup_page_table)) {
			pte = list_entry(e, struct sup_pte, sup_elem);
			e = list_next(e);
			free(pte);
		}
	} 
}

// Calls pagedir_set_accessed to set the accessed bit for a given upage
void page_set_accessed(uint32_t* pd, void* upage, bool accessed) {
	/*Rebecca Drove Here*/
	pagedir_set_accessed(pd, upage, accessed);
}

// Returns true if the accessed bit is set, false otherwise.
bool page_is_accessed(uint32_t* pd, void* upage) {
	/*Rebecca Drove Here*/
	bool accessed = pagedir_is_accessed(pd, upage);
	return accessed;
}
