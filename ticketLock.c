#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "ticketlock.h"

void
initlock_t(struct ticketlock *lk, char *name)
{
  lk->name = name;
  lk->proc = 0;
  lk->ticket = 0;
  lk->turn = 0;
}

// Acquire the lock.
void
acquire_t(struct ticketlock *lk)
{
  uint ticket;
  //pushcli(); // disable interrupts to avoid deadlock.
  if(holding_t(lk))
    panic("acquire");

  ticket = fetch_and_add(&lk->ticket, 1);
  while(lk->turn != ticket)
    givepriority(lk->proc);

  // Record info about lock acquisition for debugging.
  lk->cpu = mycpu();
  lk->proc = myproc();
  getcallerpcs(&lk, lk->pcs);
}

// Release the lock.
void
release_t(struct ticketlock *lk)
{
  if(!holding_t(lk))
    panic("release");

  lk->pcs[0] = 0;
  lk->proc = 0;
  lk->cpu = 0;

  lk->turn++; //fetch_and_add(&lk->turn, 1);
  wakeup(lk);
  resetpriority();

  //popcli();
}

// Check whether this cpu is holding the lock.
int
holding_t(struct ticketlock *lock)
{
  return (lock->ticket != lock->turn) && (lock->proc == myproc());
}