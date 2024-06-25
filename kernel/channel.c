// channel.c

#include "types.h"
#include "defs.h"
#include "channel.h"

// Define the array of channels globally
struct channel channels[NCHANNEL];

void channelinit(void) {
  for (int i = 0; i < NCHANNEL; i++) {
    initlock(&channels[i].lock, "channel");
    channels[i].is_valid = 0;
    channels[i].is_empty = 1;
    channels[i].owner = 0;
  }
}
