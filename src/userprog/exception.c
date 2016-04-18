#include "userprog/exception.h"
#include <inttypes.h>
#include <stdio.h>
#include "userprog/gdt.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/syscall.h"
#include "vm/frame.h"
#include "vm/page.h"
#include "threads/vaddr.h"
#include "filesys/file.h"
#include <string.h>
#include "userprog/pagedir.h"
#include "vm/swap.h"
#include "devices/block.h"


/* Number of page faults processed. */
static long long page_fault_cnt;

static void kill (struct intr_frame *);
static void page_fault (struct intr_frame *);

/* Registers handlers for interrupts that can be caused by user
   programs.

   In a real Unix-like OS, most of these interrupts would be
   passed along to the user process in the form of signals, as
   described in [SV-386] 3-24 and 3-25, but we don't implement
   signals.  Instead, we'll make them simply kill the user
   process.

   Page faults are an exception.  Here they are treated the same
   way as other exceptions, but this will need to change to
   implement virtual memory.

   Refer to [IA32-v3a] section 5.15 "Exception and Interrupt
   Reference" for a description of each of these exceptions. */
void
exception_init (void) 
{
  /* These exceptions can be raised explicitly by a user program,
     e.g. via the INT, INT3, INTO, and BOUND instructions.  Thus,
     we set DPL==3, meaning that user programs are allowed to
     invoke them via these instructions. */
  intr_register_int (3, 3, INTR_ON, kill, "#BP Breakpoint Exception");
  intr_register_int (4, 3, INTR_ON, kill, "#OF Overflow Exception");
  intr_register_int (5, 3, INTR_ON, kill,
                     "#BR BOUND Range Exceeded Exception");

  /* These exceptions have DPL==0, preventing user processes from
     invoking them via the INT instruction.  They can still be
     caused indirectly, e.g. #DE can be caused by dividing by
     0.  */
  intr_register_int (0, 0, INTR_ON, kill, "#DE Divide Error");
  intr_register_int (1, 0, INTR_ON, kill, "#DB Debug Exception");
  intr_register_int (6, 0, INTR_ON, kill, "#UD Invalid Opcode Exception");
  intr_register_int (7, 0, INTR_ON, kill,
                     "#NM Device Not Available Exception");
  intr_register_int (11, 0, INTR_ON, kill, "#NP Segment Not Present");
  intr_register_int (12, 0, INTR_ON, kill, "#SS Stack Fault Exception");
  intr_register_int (13, 0, INTR_ON, kill, "#GP General Protection Exception");
  intr_register_int (16, 0, INTR_ON, kill, "#MF x87 FPU Floating-Point Error");
  intr_register_int (19, 0, INTR_ON, kill,
                     "#XF SIMD Floating-Point Exception");

  /* Most exceptions can be handled with interrupts turned on.
     We need to disable interrupts for page faults because the
     fault address is stored in CR2 and needs to be preserved. */
  intr_register_int (14, 0, INTR_OFF, page_fault, "#PF Page-Fault Exception");
}

/* Prints exception statistics. */
void
exception_print_stats (void) 
{
  printf ("Exception: %lld page faults\n", page_fault_cnt);
}

/* Handler for an exception (probably) caused by a user process. */
static void
kill (struct intr_frame *f) 
{
  /* This interrupt is one (probably) caused by a user process.
     For example, the process might have tried to access unmapped
     virtual memory (a page fault).  For now, we simply kill the
     user process.  Later, we'll want to handle page faults in
     the kernel.  Real Unix-like operating systems pass most
     exceptions back to the process via signals, but we don't
     implement them. */
     
  /* The interrupt frame's code segment value tells us where the
     exception originated. */
  switch (f->cs)
    {
    case SEL_UCSEG:
      /* User's code segment, so it's a user exception, as we
         expected.  Kill the user process.  */
      printf ("%s: dying due to interrupt %#04x (%s).\n",
              thread_name (), f->vec_no, intr_name (f->vec_no));
      intr_dump_frame (f);
      exit(-1);

    case SEL_KCSEG:
      /* Kernel's code segment, which indicates a kernel bug.
         Kernel code shouldn't throw exceptions.  (Page faults
         may cause kernel exceptions--but they shouldn't arrive
         here.)  Panic the kernel to make the point.  */
      intr_dump_frame (f);
      PANIC ("Kernel bug - unexpected interrupt in kernel");

    default:
      /* Some other code segment?  Shouldn't happen.  Panic the
         kernel. */
      printf ("Interrupt %#04x (%s) in unknown segment %04x\n",
             f->vec_no, intr_name (f->vec_no), f->cs);
      thread_exit ();
    }
}

static bool
page_install (void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (t->pagedir, upage) == NULL
          && pagedir_set_page (t->pagedir, upage, kpage, writable));
}

/* Page fault handler.  This is a skeleton that must be filled in
   to implement virtual memory.  Some solutions to project 2 may
   also require modifying this code.
   At entry, the address that faulted is in CR2 (Control Register
   2) and information about the fault, formatted as described in
   the PF_* macros in exception.h, is in F's error_code member.  The
   example code here shows how to parse that information.  You
   can find more information about both of these in the
   description of "Interrupt 14--Page Fault Exception (#PF)" in
   [IA32-v3a] section 5.15 "Exception and Interrupt Reference". */
static void
page_fault (struct intr_frame *f) 
{
  bool not_present;  /* True: not-present page, false: writing r/o page. */
  bool write;        /* True: access was write, false: access was read. */
  bool user;         /* True: access by user, false: access by kernel. */
  void *fault_addr;  /* Fault address. */

  /* Obtain faulting address, the virtual address that was
     accessed to cause the fault.  It may point to code or to
     data.  It is not necessarily the address of the instruction
     that caused the fault (that's f->eip).
     See [IA32-v2a] "MOV--Move to/from Control Registers" and
     [IA32-v3a] 5.15 "Interrupt 14--Page Fault Exception
     (#PF)". */
  asm ("movl %%cr2, %0" : "=r" (fault_addr));

  /* Turn interrupts back on (they were only off so that we could
     be assured of reading CR2 before it changed). */
  intr_enable ();

  /* Count page faults. */
  page_fault_cnt++;

  /* Determine cause. */
  not_present = (f->error_code & PF_P) == 0;
  write = (f->error_code & PF_W) != 0;
  user = (f->error_code & PF_U) != 0;
  
  /*Rebecca Drove Here*/
  void* upage;
  void* old_upage;
  struct sup_pte* pte;
  uint8_t* kpage;
  void* esp;
  uint32_t* owner_pagedir;
  struct thread* owner;
  struct frame* frame;

  if(fault_addr == NULL)
    kill(f);

  if (!user)
    esp = thread_current()->esp;
  else
    esp = f->esp;

  if(not_present) {
    lock_acquire(&evict_lock);
    upage = pg_round_down(fault_addr);
    kpage = frame_alloc(upage);

    /*Jorge Drove Here*/
    // 0. Check if frame table is full
    if(kpage == NULL) { // evict first
      kpage = frame_evict();               // evict frame
      frame = frame_get_frame(kpage);
      old_upage = frame->upage;
      owner = frame->owner;
      owner_pagedir = frame->pd;
      frame_dealloc(kpage);                // free frame
      if(pagedir_is_dirty(owner_pagedir, kpage))
        swap_add(kpage, old_upage, owner); // add to swap table
      frame_set(upage, kpage);             // set new upage to frame
    }
    else
      frame = frame_get_frame(kpage);

    /*Charles Drove Here*/
    // 1. Get page table entry
    pte = page_get(upage);
    if(pte == NULL) { // if null check for stack growth or exit
      if ((fault_addr < PHYS_BASE) && (fault_addr >= (esp - 32))) { // stack growth
        pte = malloc(sizeof(struct sup_pte));
        page_add_sp(pte, upage);  // add new entry to sup page table
      }
     else {
        lock_release(&evict_lock);
        exit(-1);
      }
    }
    else { // if not null.. check if in swap first, else file read
      /*Jasmine and Rebecca Drove Here*/
      // 2. Check swap table
      if(swap_remove(kpage, upage)) { // read from swap if exists
        pagedir_set_dirty(thread_current()->pagedir, kpage, 1); // set dirty bit
      }
      // 3. Load this page
      else { // read from file
        file_seek (pte->file, pte->offset);

        if (file_read (pte->file, kpage, pte->page_read_bytes) != 
          (int) pte->page_read_bytes) {
          kill(f);
        }
        memset (kpage + pte->page_read_bytes, 0, pte->page_zero_bytes);
      }
    }

    // 4. Map page with frame
    /* Add the page to the process's address space. */
    // move this to page_fault (lazy loading)
    if (!page_install (upage, kpage, pte->writable)) {
      kill(f); 
    }
    lock_release(&evict_lock);
  }
  else {
    exit(-1);
  }

  /* To implement virtual memory, delete the rest of the function
     body, and replace it with code that brings in the page to
     which fault_addr refers. */
  /*printf ("Page fault at %p: %s error %s page in %s context.\n",
         fault_addr,
         not_present ? "not present" : "rights violation",
         write ? "writing" : "reading",
         user ? "user" : "kernel");*/

  //printf("There is no crying in Pintos!\n");
}
