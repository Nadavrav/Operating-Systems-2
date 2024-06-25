#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "channel.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


int sys_channel_create(void) {
  for (int i = 0; i < NCHANNEL; i++) {
    struct channel *ch = &channels[i];
    acquire(&ch->lock); // Acquire lock to ensure exclusive access
    if (!ch->is_valid) { // Find an available (invalid) channel
      ch->is_valid = 1; // Mark the channel as valid
      ch->is_empty = 1; // Initialize as empty
      ch->owner = myproc(); // Set the owner to the current process
      release(&ch->lock); // Release lock after initialization
      return i; // Return the channel descriptor (index)
    }
    release(&ch->lock); // Release lock if the channel is not available
  }
  return -1; // Return -1 if no available channel is found
}

int sys_channel_put(void) {
  int cd, data;
  
  // Use argint to fetch the arguments without checking for return values
  argint(0, &cd); 
  argint(1, &data);

  // Validate the channel descriptor
  if (cd < 0 || cd >= NCHANNEL)
    return -1; // Invalid channel descriptor

  struct channel *ch = &channels[cd];
  acquire(&ch->lock); // Acquire lock to access the channel
  if (!ch->is_valid) {
    release(&ch->lock);
    return -1; // Check if the channel is valid
  }

  // Wait until the channel is empty
  while (!ch->is_empty) {
    sleep(ch, &ch->lock); // Sleep until the channel is empty
    if (!ch->is_valid) {  // Check if the channel was destroyed while sleeping
      release(&ch->lock);
      return -1;
    }
  }

  ch->data = data; // Place data in the channel
  ch->is_empty = 0; // Mark the channel as not empty
  wakeup(ch); // Wake up any processes waiting to take data

  release(&ch->lock); // Release lock
  return 0;
}

int sys_channel_take(void) {
  int cd;
  int *user_data;

  // Use argint and argptr to fetch arguments
  argint(0, &cd);
  argptr(1, (char **)&user_data, sizeof(int));

  // Validate the channel descriptor
  if (cd < 0 || cd >= NCHANNEL)
    return -1; // Invalid channel descriptor

  struct channel *ch = &channels[cd];
  acquire(&ch->lock); // Acquire lock to access the channel
  if (!ch->is_valid) {
    release(&ch->lock);
    return -1; // Check if the channel is valid
  }

  // Wait until the channel has data
  while (ch->is_empty) {
    sleep(ch, &ch->lock); // Sleep until the channel has data
    if (!ch->is_valid) {  // Check if the channel was destroyed while sleeping
      release(&ch->lock);
      return -1;
    }
  }

  int kernel_data = ch->data; // Retrieve data from the channel
  ch->is_empty = 1; // Mark the channel as empty
  wakeup(ch); // Wake up any processes waiting to put data

  // Copy data to user space
  if (copyout(myproc()->pagetable, (uint64)user_data, (char *)&kernel_data, sizeof(int)) < 0) {
    release(&ch->lock);
    return -1;
  }

  release(&ch->lock); // Release lock
  return 0;
}

int sys_channel_destroy(void) {
  int cd;
  
  // Use argint to fetch the argument
  argint(0, &cd);

  // Validate the channel descriptor
  if (cd < 0 || cd >= NCHANNEL)
    return -1; // Invalid channel descriptor

  struct channel *ch = &channels[cd];
  acquire(&ch->lock); // Acquire lock to access the channel
  if (!ch->is_valid) {
    release(&ch->lock);
    return -1; // Check if the channel is already invalid
  }

  ch->is_valid = 0; // Mark the channel as invalid
  wakeup(ch); // Wake up any processes waiting on this channel

  release(&ch->lock); // Release lock
  return 0;
}
