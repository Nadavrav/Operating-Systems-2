#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include "spinlock.h"  // Include the spinlock definition

// Define maximum number of channels supported
#define MAX_CHANNELS 10  // Adjust as needed

// Channel structure
struct channel {
    struct spinlock lock;  // Spinlock to protect channel data
    int data;              // Data item in the channel (for simplicity, int type)
    int data_available;    // Flag indicating if data is available
    // Add more fields as needed, e.g., creator process ID
};

extern struct channel channels[MAX_CHANNELS];  // Global array of channels

#endif // _CHANNEL_H_
