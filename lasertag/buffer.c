#include "buffer.h"
#include <stdlib.h>
#include <assert.h>

#define EMPTY 0
#define BUFFER_SIZE 32768

#define MALLOC_ERR "ERROR_INIT_DATA_MALLOC_FAILED"
#define OVERFLOW_ERR "ERROR_OVERFLOW"
#define UNDERFLOW_ERR "ERROR_UNDERFLOW"
#define INDEX_OUT_OF_RANGE_ERR "ERROR_INDEX_OUT_OF_RANGE"

// This implements a dedicated circular buffer for storing values
// from the ADC until they are read and processed by the detector.
// The function of the buffer is similar to a queue or FIFO.

// Type of elements in the buffer.
typedef uint32_t buffer_data_t;

typedef struct {
    uint32_t indexIn; // Points to the next open slot.
    uint32_t indexOut; // Points to the next element to be removed.
    uint32_t elementCount; // Number of elements in the buffer.
    buffer_data_t data[BUFFER_SIZE]; // Values are stored here.
} buffer_t;

volatile static buffer_t buf; //Buffer variable

// Initialize the buffer to empty.
void buffer_init(void){

    //intializing all the variables that are 0
    buf.indexIn = 0;
    buf.indexOut = 0;

    //adding last variables
    buf.elementCount = EMPTY;
}

// Add a value to the buffer. Overwrite the oldest value if full.
void buffer_pushover(buffer_data_t value){

    //If buffer full, overwrite buffer value
    if(buf.elementCount == BUFFER_SIZE){
        buf.elementCount--;
        buf.indexOut = (buf.indexOut+1) % BUFFER_SIZE;
    }


    //Increment Element count, write value to indexIn, increment indexIn
    buf.data[buf.indexIn] = value;
    buf.elementCount++;
    buf.indexIn = (buf.indexIn+1) % BUFFER_SIZE;
}

// Remove a value from the buffer. Return zero if empty.
buffer_data_t buffer_pop(void){

    // Return 0 if buffer empty
    if(buf.elementCount == 0){
        return EMPTY;
    }

    // Changing things that are done only when the buffer is empty
    buffer_data_t temp_data = buf.data[buf.indexOut];
    buf.elementCount--;
    buf.indexOut = (buf.indexOut+1) % BUFFER_SIZE;
    // Return buffer_data_t type
    return temp_data;
}

// Return the number of elements in the buffer.
uint32_t buffer_elements(void) {
    return buf.elementCount;
}

// Return the capacity of the buffer in elements.
uint32_t buffer_size(void) {
    return BUFFER_SIZE;
}

