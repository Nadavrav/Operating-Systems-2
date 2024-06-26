#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "channel.h"


struct channel channels[MAX_CHANNELS];
static struct spinlock channels_lock;

void
channel_init(void)
{
    initlock(&channels_lock, "channels");
}

int
channel_create(void)
{
    acquire(&channels_lock);

    // Find a free slot in the channels array
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (!channels[i].lock.locked) {
            initlock(&channels[i].lock, "channel");
            channels[i].data_available = 0;  // Initialize to no data available
            // Initialize other fields as needed
            release(&channels_lock);
            return i;  // Return the channel descriptor (index)
        }
    }

    release(&channels_lock);
    return -1;  // No free channels available
}

int
channel_put(int cd, int data)
{
    struct channel *ch;

    if (cd < 0 || cd >= MAX_CHANNELS)
        return -1;

    ch = &channels[cd];
    acquire(&ch->lock);

    while (ch->data_available) {
        sleep(ch, &ch->lock);
    }

    ch->data = data;
    ch->data_available = 1;

    wakeup(ch);

    release(&ch->lock);

    return 0;
}

int
channel_take(int cd, int *data)
{
    struct channel *ch;

    if (cd < 0 || cd >= MAX_CHANNELS)
        return -1;

    ch = &channels[cd];
    acquire(&ch->lock);

    while (!ch->data_available) {
        sleep(ch, &ch->lock);
    }

    *data = ch->data;
    ch->data_available = 0;

    wakeup(ch);

    release(&ch->lock);

    return 0;
}

int
channel_destroy(int cd)
{
    struct channel *ch;

    if (cd < 0 || cd >= MAX_CHANNELS)
        return -1;

    ch = &channels[cd];
    acquire(&ch->lock);

    // Clear channel data and mark as unavailable
    ch->data = 0;
    ch->data_available = 0;

    // Signal any sleeping processes
    wakeup(ch);

    release(&ch->lock);

    // Destroy the lock (optional)
    // dealloclock(&ch->lock);

    return 0;
}
