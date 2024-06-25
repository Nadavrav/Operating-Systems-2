// channel.h (New header file for channel definition)
#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include "types.h"
#include "spinlock.h"

// Maximum number of channels
#define NCHANNEL 64
//extern struct channel channels[NCHANNEL];

struct channel {
  struct spinlock lock;  // Spinlock for synchronization
  int data;              // Data item
  int is_empty;          // 1 if channel is empty, 0 if it has data
  int is_valid;          // 1 if the channel is valid, 0 otherwise
  struct proc *owner;    // Process that created the channel
};

#endif // _CHANNEL_H_
