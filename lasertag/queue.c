#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

//STANDARD VALUES
#define TRUE 1
#define FALSE 0

//ERROR MESSAGES
#define MALLOC_ERR "ERROR_INIT_DATA_MALLOC_FAILED"
#define OVERFLOW_ERR "ERROR_OVERFLOW"
#define UNDERFLOW_ERR "ERROR_UNDERFLOW"
#define INDEX_OUT_OF_RANGE_ERR "ERROR_INDEX_OUT_OF_RANGE"

//QUEUE STUFF
#define EMPTY 0 


// Allocates memory for the queue (the data* pointer) and initializes all
// parts of the data structure. Prints out an error message if malloc() fails
// and calls assert(false) to print-out line-number information and die.
// The queue is empty after initialization. To fill the queue with known
// values (e.g. zeros), call queue_overwritePush() up to queue_size() times.
void queue_init(queue_t *q, queue_size_t size, const char *name){
    
    //Malloc for data*
    if(!(q->data = (queue_data_t*)malloc(size*sizeof(queue_data_t)))){
        printf("%s\n",MALLOC_ERR);
        assert(FALSE);
    }

    //intializing all the variables that are 0
    q->indexIn = 0;
    q->indexOut = 0;

    //adding last variables
    q->elementCount = EMPTY;
    q->size = size;

    //under and over flow flags
    q->underflowFlag = FALSE;
    q->overflowFlag = FALSE;
    strcpy(q->name,name);
}


// Get the user-assigned name for the queue.
const char *queue_name(queue_t *q){
    return q->name;
}

// Returns the capacity of the queue.
queue_size_t queue_size(queue_t *q){
    return q->size;
}

// Returns true if the queue is full.
bool queue_full(queue_t *q){
    return (q->elementCount == q->size);
}

// Returns true if the queue is empty.
bool queue_empty(queue_t *q){
    return q->elementCount == EMPTY;
}

// If the queue is not full, pushes a new element into the queue and clears the
// underflowFlag. IF the queue is full, set the overflowFlag, print an error
// message and DO NOT change the queue.
void queue_push(queue_t *q, queue_data_t value){
    //doing only what needs to be done if the queue is full
    if (queue_full(q))
    {
        q->overflowFlag = TRUE;
        printf("%s\n",OVERFLOW_ERR);
        //return so the other parts of this function are not run
        return;
    }

    //changing things that are done only when the queue is not full
    q->data[q->indexIn] = value;
    q->elementCount++;
    q->indexIn = (q->indexIn+1) % q->size;

    //make sure the flag is cleared
    q->underflowFlag = FALSE;
}

// If the queue is not empty, remove and return the oldest element in the queue.
// If the queue is empty, set the underflowFlag, print an error message, and DO
// NOT change the queue.
queue_data_t queue_pop(queue_t *q){
    //specific instructions for only if the queue is not empty 
    if(queue_empty(q)){
        q->underflowFlag = TRUE;
        printf("%s\n",UNDERFLOW_ERR);
        return FALSE;
    }

    //changing things that are done only when the queue is empty
    queue_data_t temp_data = q->data[q->indexOut];
    q->elementCount--;
    q->indexOut = (q->indexOut+1) % q->size;
    q->overflowFlag = FALSE;

    //return queue_data_t type
    return temp_data;
}

// If the queue is full, call queue_pop() and then call queue_push().
// If the queue is not full, just call queue_push().
void queue_overwritePush(queue_t *q, queue_data_t value){
    if(queue_full(q)){
        queue_pop(q);
    }
    queue_push(q,value);
}

// Provides random-access read capability to the queue.
// Low-valued indexes access older queue elements while higher-value indexes
// access newer elements (according to the order that they were added). Print a
// meaningful error message if an error condition is detected.
queue_data_t queue_readElementAt(queue_t *q, queue_index_t index){
    if (q->elementCount - 1 < index || index < 0)
    {
        printf("%s\n",INDEX_OUT_OF_RANGE_ERR);
        return FALSE;
    }

    //return queue_data_t type
    return q->data[(q->indexOut+index) % q->size];
}

// Returns a count of the elements currently contained in the queue.
queue_size_t queue_elementCount(queue_t *q){
    return q->elementCount;
}

// Returns true if an underflow has occurred (queue_pop() called on an empty
// queue).
bool queue_underflow(queue_t *q){
    return q->underflowFlag;
}

// Returns true if an overflow has occurred (queue_push() called on a full
// queue).
bool queue_overflow(queue_t *q){
    return q->overflowFlag;
}

// Frees the storage that you malloc'd before.
void queue_garbageCollect(queue_t *q){
    free(q->data);
}