#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {
//you write this
  int i = 0;
  int found = 0;
  struct proc *curproc = myproc();
  acquire(&(shm_table.lock));
  for (i = 0; i < 64; i++) {
    if(shm_table.shm_pages[i].id == id) {
      found = 1;
      break;
    }
  }
  if (found) { //Case1:
    mappages(curproc->pgdir, (void*)PGROUNDUP(curproc->sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
    shm_table.shm_pages[i].refcnt = shm_table.shm_pages[i].refcnt + 1;
    *pointer = (char *)PGROUNDUP(curproc->sz);
    curproc->sz = PGROUNDUP(curproc->sz) + PGSIZE;
    release(&(shm_table.lock));
    return 0;
  }
  else { //Case2:
    for (i = 0; i < 64; i++) {
      if(shm_table.shm_pages[i].refcnt == 0) 
        break; 
    }
    shm_table.shm_pages[i].id = id;
    char *mem;
    mem = kalloc();
    if (mem == 0) {
      cprintf("allocuvm out of memory\n");
      deallocuvm(curproc->pgdir, 0, curproc->sz);
      release(&(shm_table.lock));
      return 0;
    } 
    else {
      memset(mem, 0, PGSIZE);
      shm_table.shm_pages[i].frame = mem;
      if(mappages(curproc->pgdir, (void*)PGROUNDUP(curproc->sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U) < 0) {
        deallocuvm(curproc->pgdir, 0, curproc->sz);
        kfree(shm_table.shm_pages[i].frame);
        release(&(shm_table.lock));
        return 0;
      }
      else {
         shm_table.shm_pages[i].refcnt = shm_table.shm_pages[i].refcnt + 1;
         *pointer = (char *)PGROUNDUP(curproc->sz);
         curproc->sz = PGROUNDUP(curproc->sz) + PGSIZE;
         release(&(shm_table.lock));
         return 0;
      }
    }
  }
  //return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
//you write this too!
  int i;
  acquire(&(shm_table.lock));
  for(i = 0; i < 64; i++) {
    if(shm_table.shm_pages[i].id == id) {
      if(shm_table.shm_pages[i].refcnt >= 1) {
        shm_table.shm_pages[i].refcnt--;
      }
    else {
      shm_table.shm_pages[i].id = 0;
      kfree(shm_table.shm_pages[i].frame);
      shm_table.shm_pages[i].refcnt = 0;
      }
    }
  }
  release(&(shm_table.lock));

  return 0; //added to remove compiler warning -- you should decide what to return
}
