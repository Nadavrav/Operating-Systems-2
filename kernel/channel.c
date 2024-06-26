#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "proc.h"
#include "channel.h"

void
channelinit(void)
{
  for(int i = 0; i < NCHANNEL; i++){
    channels[i].used = 0;
    channels[i].has_data = 0;
    initlock(&channels[i].lock, "channel");
    channels[i].owner=0;
  }
}

int
channel_create(void) {
    for(int i = 0; i < NCHANNEL; i++) {
        acquire(&channels[i].lock);
        if(channels[i].owner == 0) {
            channels[i].owner = myproc();
            release(&channels[i].lock);
            return i;
        }
        release(&channels[i].lock);
    }
    return -1;
}

int
channel_put(int cd, int data) {
    if(cd < 0 || cd >= NCHANNEL)
        return -1;

    acquire(&channels[cd].lock);
    while(channels[cd].has_data) {
        sleep(&channels[cd], &channels[cd].lock);
    }

    channels[cd].data = data;
    channels[cd].has_data = 1;
    wakeup(&channels[cd]);
    release(&channels[cd].lock);
    return 0;
}

int
channel_take(int cd, int *data) {
    if(cd < 0 || cd >= NCHANNEL)
        return -1;

    acquire(&channels[cd].lock);
    while(!channels[cd].has_data) {
        sleep(&channels[cd], &channels[cd].lock);
    }

    *data = channels[cd].data;
    channels[cd].has_data = 0;
    wakeup(&channels[cd]);
    release(&channels[cd].lock);
    return 0;
}

int
channel_destroy(int cd) {
    if(cd < 0 || cd >= NCHANNEL)
        return -1;

    acquire(&channels[cd].lock);
    if(channels[cd].owner != myproc()) {
        release(&channels[cd].lock);
        return -1;
    }

    channels[cd].owner = 0;
    channels[cd].has_data = 0;
    wakeup(&channels[cd]);
    release(&channels[cd].lock);
    return 0;
}