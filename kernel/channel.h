//#ifndef CHANNEL_H
//#define CHANNEL_H

#include "spinlock.h"
#include "proc.h"

struct channel {
  int used;           // Is this channel in use?
  int data;           // The data item in the channel
  int has_data;       // Is there data in the channel?
  struct spinlock lock; // Lock to protect this channel
  struct proc *owner; // The process that created this channel
};

//#define NCHANNEL 10  // Maximum number of channels

extern struct channel channels[NCHANNEL];

// Channel functions
void channelinit(void);
int channel_create(void);
int channel_put(int cd, int data);
int channel_take(int cd, int *data);
int channel_destroy(int cd);

//#endif // CHANNEL_H
