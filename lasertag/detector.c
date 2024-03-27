#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "buffer.h"
#include "filter.h"
#include "lockoutTimer.h"
#include "hitLedTimer.h"
#include "interrupts.h"
#include "invincibilityTimer.h"


#define FREQUENCY_COUNT 10
#define COUNT_BEFORE_FILTER 10
#define ADC_SCALAR 4.8840048E-4
#define FUDGE_FACTOR 190
#define MEDIAN_INDEX 4
#define NO_HIT_DETECTED -1

#define TEST_POWER_VALUE_SET_1 1.1,2.2,4.1,100000,3.5,2.6,2,5,1.2,.04
#define TEST_POWER_VALUE_SET_2 100.2,50.4,4.1,402.5,3.5,20.5,2,5,2.53,204.3


typedef uint16_t detector_hitCount_t;
bool ignoredFreq[FREQUENCY_COUNT];
uint16_t adcValuesAdded;
volatile bool detector_hitDetectedFlag;
uint32_t detector_hitArray[FREQUENCY_COUNT];
uint16_t lastHit;
uint64_t invocationCount;
bool first_run;


//hit_detect function that determins if there has been a registered
//hit, and if there has been, set the hitDetected flag to true
//lastHit to the filter the hit wsa registered on.
void hit_detect(){

    //Create Arrays to Hold 1) Power Values, 2) sorted list of powerValues indexes
    uint16_t filterSorted[FREQUENCY_COUNT];
    double powerValues[FREQUENCY_COUNT];

    //Copy current power values from filter
    filter_getCurrentPowerValues(powerValues);

    filterSorted[0] = 0;

    //Nested for loops to do insertion sort
    for (uint16_t insert_filter = 1; insert_filter < FREQUENCY_COUNT; insert_filter++){
        filterSorted[insert_filter] = insert_filter;

        //Inner loop to put filter number into sorted place
        for(uint16_t compare_filter = insert_filter-1; compare_filter >= 0; compare_filter--){

            //If the value is greater or equal to the one before it, do not switch it
            if(powerValues[insert_filter] >= powerValues[filterSorted[compare_filter]]){
                break;   
            }

            //Else switch the compare filter and the insert_filter
            filterSorted[compare_filter+1] = filterSorted[compare_filter];
            filterSorted[compare_filter] = insert_filter;
        }
    }

    //If there is a shot detected, set the hitDetectedFlag to true and register the cordinating filter to the lastHit
    if(!ignoredFreq[filterSorted[FREQUENCY_COUNT-1]] && (powerValues[filterSorted[FREQUENCY_COUNT - 1]] >= FUDGE_FACTOR*powerValues[filterSorted[MEDIAN_INDEX]])){
        detector_hitDetectedFlag = TRUE; //Set hitDetectedFlag to true
        lastHit = filterSorted[FREQUENCY_COUNT-1]; //Set lastHit to the registered hit filter
    }

}


// Initialize the detector module.
// By default, all frequencies are considered for hits.
// Assumes the filter module is initialized previously.
void detector_init(void) {

    //Iterate through ignored frequencies setting all to false
    for (uint16_t i = 0; i < FREQUENCY_COUNT; i++){
        ignoredFreq[i] = FALSE;
    }
    filter_init();
    //Assert asvValuesAdded to 0 and detector_hitDetectedFlag to false
    adcValuesAdded = 0;
    detector_hitDetectedFlag = FALSE;
    first_run = true;
}

// freqArray is indexed by frequency number. If an element is set to true,
// the frequency will be ignored. Multiple frequencies can be ignored.
// Your shot frequency (based on the switches) is a good choice to ignore.
void detector_setIgnoredFrequencies(bool freqArray[]) {
    //Iterate through ignored frequencies copying new data in
    for (uint16_t i = 0; i < FREQUENCY_COUNT; i++){
        ignoredFreq[i] = freqArray[i];
    }
}

// Returns true if a hit was detected.
bool detector_hitDetected(void) {
    return detector_hitDetectedFlag;
}

// Runs the entire detector: decimating FIR-filter, IIR-filters,
// power-computation, hit-detection. If interruptsCurrentlyEnabled = true,
// interrupts are running. If interruptsCurrentlyEnabled = false you can pop
// values from the ADC buffer without disabling interrupts. If
// interruptsCurrentlyEnabled = true, do the following:
// 1. disable interrupts.
// 2. pop the value from the ADC buffer.
// 3. re-enable interrupts.
// Ignore hits on frequencies specified with detector_setIgnoredFrequencies().
// Assumption: draining the ADC buffer occurs faster than it can fill.
void detector(bool interruptsCurrentlyEnabled) {
    invocationCount++; //Increment filter invocation count
    uint32_t bufferElements = buffer_elements(); //read in bufferelement count

    //For each buffer element
        //read the raw value
        //create a scaled value
        //if it is the 10th element
            //run all filters and power calculations
            //determine if a hit has been registered
            //if a hit has been registered, increment that hit count
    for(uint32_t i = bufferElements; i > 0; --i){
        
        if(interruptsCurrentlyEnabled) // If interruptsCurrentlyEnabled, disable the arm interrupts while reading values 
            interrupts_disableArmInts();
        
        buffer_data_t rawAdcValue = buffer_pop();

        if(interruptsCurrentlyEnabled)// If interruptsCurrentlyEnabled, enable the arm interrupts after reading values 
            interrupts_enableArmInts();

        double scaledAdcValue = (rawAdcValue * (ADC_SCALAR - 1)); //Change the ADC value to a number between -1 and 1

        filter_addNewInput(scaledAdcValue);  //Add the value to the filters
        adcValuesAdded++; //Increment the number of values added
        
        if (adcValuesAdded == COUNT_BEFORE_FILTER) {
            //Run Filters
            filter_firFilter();

            //For each filter 0-9, run iir_filter and power calulation
            for (uint16_t filter = 0; filter < FILTER_FREQUENCY_COUNT; ++filter){
                filter_iirFilter(filter);
                filter_computePower(filter, first_run, FALSE);
                first_run = false;
            }

            //Run if lockout Timer is Not Running
            if (!lockoutTimer_running() && !invincibilityTimer_running()){
                hit_detect(); //Run hit_detect() algorithm
                if(detector_hitDetected()){
                    lockoutTimer_start(); //Start the lockout timer
                    hitLedTimer_start(); //Start the hit LED timer
                    detector_hitArray[lastHit]++; //Increment the count of the filter that registered the hit
                }
            }
            adcValuesAdded = 0; //Reset the adc added counter
        }
    }
}


// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit(void) {
    return lastHit;
}

// Clear the detected hit once you have accounted for it.
void detector_clearHit(void) {
    detector_hitDetectedFlag = false;
}

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
void detector_ignoreAllHits(bool flagValue) {
    return;
}

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]) {

    //For each element in the filter array, return the registered hits
    for (uint32_t i = 0; i < FREQUENCY_COUNT; i++) {
       hitArray[i] = detector_hitArray[i];
    }
}

// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t factorIdx) {
    return;
}

// Returns the detector invocation count.
// The count is incremented each time detector is called.
// Used for run-time statistics.
uint32_t detector_getInvocationCount(void) {
    return invocationCount;
}

/******************************************************
******************** Test Routines ********************
******************************************************/

// Students implement this as part of Milestone 3, Task 3.
// Create two sets of power values and call your hit detection algorithm
// on each set. With the same fudge factor, your hit detect algorithm
// should detect a hit on the first set and not detect a hit on the second.
void detector_runTest(void) {

    printf("Testing with Fudge number %d\n\nExpected hit on filter 1\n",FUDGE_FACTOR);

    double powerValues1[FILTER_FREQUENCY_COUNT] = {TEST_POWER_VALUE_SET_1}; //hit detection test with 1 hit

    printf("Testing Values: ");

    //Iterate through powerValues set and add each value to corresponding filter power
    for(uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; ++i){
    filter_setCurrentPowerValue(i,powerValues1[i]);
    printf("%f ",powerValues1[i]);
    }
    printf("\n"); //Print new line

    hit_detect(); //Run the hit detection algorithm

    //Print either hit dectected or not
    if (detector_hitDetected()){
        printf("Hit Detected on %d\n",lastHit);
    }
    else{
        printf("No Hit Detected\n");
    }

    detector_clearHit(); //Clear hit

    printf("Testing with Fudge number %d\n\nNo Expected Hit\n",FUDGE_FACTOR);


    double powerValues2[FILTER_FREQUENCY_COUNT] = {TEST_POWER_VALUE_SET_2}; //Array for hit detection test with no hits

    printf("Testing Values: ");
    
    //Iterate through powerValues set and add each value to corresponding filter power
    for(uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; ++i){
        filter_setCurrentPowerValue(i,powerValues2[i]);
        printf("%f ",powerValues2[i]);
    } 
    
    printf("\n"); //Print new line

    hit_detect(); //Run the hit detection algorithm

    //Print either hit dectected or not
    if (detector_hitDetected()){
        printf("Hit Detected on %d\n",lastHit);
    }
    else{
        printf("No Hit Detected\n");
    }
}

