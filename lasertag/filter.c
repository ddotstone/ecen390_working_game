#include <stdint.h>
#include <stdio.h>
#include "filter.h"

//define values
#define FILTER_IIR_FILTER_COUNT 10
#define FIR_FILTER_COEFFICIENT_COUNT 81
#define IIR_A_COEFFICIENT_COUNT 10
#define IIR_B_COEFFICIENT_COUNT 11
#define QUEUE_INIT_VALUE 0.0
#define DECIMATION_VALUE 10

//queue sizes
#define Z_QUEUE_SIZE IIR_A_COEFFICIENT_COUNT
#define Y_QUEUE_SIZE IIR_B_COEFFICIENT_COUNT
#define X_QUEUE_SIZE FIR_FILTER_COEFFICIENT_COUNT
#define OUTPUT_QUEUE_SIZE 2000
#define FIR_FILTER_TAP_COUNT FIR_FILTER_COEFFICIENT_COUNT

//fir Coefficients array
const static double firCoefficients[FIR_FILTER_TAP_COUNT] = {
5.3751585173668532e-04, 
4.1057821244099187e-04, 
2.3029811615433415e-04, 
-1.0022421255268634e-05, 
-3.1239220025873498e-04, 
-6.6675539469892989e-04, 
-1.0447674325821804e-03, 
-1.3967861519587053e-03, 
-1.6537303925483089e-03, 
-1.7346382015025667e-03, 
-1.5596484449522630e-03, 
-1.0669170920384048e-03, 
-2.3088291222664008e-04, 
9.2146384892950099e-04, 
2.2999023467067978e-03, 
3.7491095163012366e-03, 
5.0578112742500408e-03, 
5.9796411037508039e-03, 
6.2645305736409576e-03, 
5.6975395969924630e-03, 
4.1403678436423832e-03, 
1.5696670848600943e-03, 
-1.8940691237627923e-03, 
-5.9726960144609528e-03, 
-1.0235869785695425e-02, 
-1.4127694707478473e-02, 
-1.7013744396319821e-02, 
-1.8244737113263923e-02, 
-1.7230507462687290e-02, 
-1.3515937014932726e-02, 
-6.8495092580338254e-03, 
2.7646568253820039e-03, 
1.5039019355364032e-02, 
2.9404269200373489e-02, 
4.5042275861018187e-02, 
6.0948410493762414e-02, 
7.6017645500231018e-02, 
8.9145705550443280e-02, 
9.9334411853457705e-02, 
1.0578952596388017e-01, 
1.0800000000000000e-01, 
1.0578952596388017e-01, 
9.9334411853457705e-02, 
8.9145705550443280e-02, 
7.6017645500231018e-02, 
6.0948410493762414e-02, 
4.5042275861018187e-02, 
2.9404269200373489e-02, 
1.5039019355364032e-02, 
2.7646568253820039e-03, 
-6.8495092580338254e-03, 
-1.3515937014932726e-02, 
-1.7230507462687290e-02, 
-1.8244737113263923e-02, 
-1.7013744396319821e-02, 
-1.4127694707478473e-02, 
-1.0235869785695425e-02, 
-5.9726960144609528e-03, 
-1.8940691237627923e-03, 
1.5696670848600943e-03, 
4.1403678436423832e-03, 
5.6975395969924630e-03, 
6.2645305736409576e-03, 
5.9796411037508039e-03, 
5.0578112742500408e-03, 
3.7491095163012366e-03, 
2.2999023467067978e-03, 
9.2146384892950099e-04, 
-2.3088291222664008e-04, 
-1.0669170920384048e-03, 
-1.5596484449522630e-03, 
-1.7346382015025667e-03, 
-1.6537303925483089e-03, 
-1.3967861519587053e-03, 
-1.0447674325821804e-03, 
-6.6675539469892989e-04, 
-3.1239220025873498e-04, 
-1.0022421255268634e-05, 
2.3029811615433415e-04, 
4.1057821244099187e-04, 
5.3751585173668532e-04};

//iir A Coefficients array
const static double iirACoefficientConstants[FILTER_FREQUENCY_COUNT][IIR_A_COEFFICIENT_COUNT] = {
{-5.9637727070164059e+00, 1.9125339333078287e+01, -4.0341474540744301e+01, 6.1537466875369077e+01, -7.0019717951472558e+01, 6.0298814235239249e+01, -3.8733792862566574e+01, 1.7993533279581207e+01, -5.4979061224868158e+00, 9.0332828533800469e-01},
{-4.6377947119071408e+00, 1.3502215749461552e+01, -2.6155952405269698e+01, 3.8589668330738235e+01, -4.3038990303252490e+01, 3.7812927599536991e+01, -2.5113598088113683e+01, 1.2703182701888030e+01, -4.2755083391143280e+00, 9.0332828533799747e-01},
{-3.0591317915750937e+00, 8.6417489609637492e+00, -1.4278790253808838e+01, 2.1302268283304294e+01, -2.2193853972079211e+01, 2.0873499791105424e+01, -1.3709764520609379e+01, 8.1303553577931567e+00, -2.8201643879900473e+00, 9.0332828533799880e-01},
{-1.4071749185996751e+00, 5.6904141470697542e+00, -5.7374718273676306e+00, 1.1958028362868905e+01, -8.5435280598354630e+00, 1.1717345583835968e+01, -5.5088290876998647e+00, 5.3536787286077674e+00, -1.2972519209655595e+00, 9.0332828533800047e-01},
{8.2010906117760318e-01, 5.1673756579268604e+00, 3.2580350909220925e+00, 1.0392903763919193e+01, 4.8101776408669084e+00, 1.0183724507092508e+01, 3.1282000712126754e+00, 4.8615933365571991e+00, 7.5604535083144919e-01, 9.0332828533800047e-01},
{2.7080869856154530e+00, 7.8319071217995795e+00, 1.2201607990980769e+01, 1.8651500443681677e+01, 1.8758157568004620e+01, 1.8276088095999114e+01, 1.1715361303018966e+01, 7.3684394621254015e+00, 2.4965418284512091e+00, 9.0332828533801224e-01},
{4.9479835250075892e+00, 1.4691607003177602e+01, 2.9082414772101060e+01, 4.3179839108869331e+01, 4.8440791644688879e+01, 4.2310703962394342e+01, 2.7923434247706432e+01, 1.3822186510471010e+01, 4.5614664160654357e+00, 9.0332828533799958e-01},
{6.1701893352279864e+00, 2.0127225876810336e+01, 4.2974193398071691e+01, 6.5958045321253465e+01, 7.5230437667866624e+01, 6.4630411355739881e+01, 4.1261591079244141e+01, 1.8936128791950541e+01, 5.6881982915180327e+00, 9.0332828533799836e-01},
{7.4092912870072398e+00, 2.6857944460290135e+01, 6.1578787811202247e+01, 9.8258255839887340e+01, 1.1359460153696304e+02, 9.6280452143026153e+01, 5.9124742025776442e+01, 2.5268527576524235e+01, 6.8305064480743178e+00, 9.0332828533800158e-01},
{8.5743055776347692e+00, 3.4306584753117903e+01, 8.4035290411037124e+01, 1.3928510844056831e+02, 1.6305115418161643e+02, 1.3648147221895812e+02, 8.0686288623299902e+01, 3.2276361903872186e+01, 7.9045143816244918e+00, 9.0332828533799903e-01}
};

//iir B Coefficients array
const static double iirBCoefficientConstants[FILTER_FREQUENCY_COUNT][IIR_B_COEFFICIENT_COUNT] = {
{9.0928661148176830e-10, 0.0000000000000000e+00, -4.5464330574088414e-09, 0.0000000000000000e+00, 9.0928661148176828e-09, 0.0000000000000000e+00, -9.0928661148176828e-09, 0.0000000000000000e+00, 4.5464330574088414e-09, 0.0000000000000000e+00, -9.0928661148176830e-10},
{9.0928661148203093e-10, 0.0000000000000000e+00, -4.5464330574101550e-09, 0.0000000000000000e+00, 9.0928661148203099e-09, 0.0000000000000000e+00, -9.0928661148203099e-09, 0.0000000000000000e+00, 4.5464330574101550e-09, 0.0000000000000000e+00, -9.0928661148203093e-10},
{9.0928661148196858e-10, 0.0000000000000000e+00, -4.5464330574098431e-09, 0.0000000000000000e+00, 9.0928661148196862e-09, 0.0000000000000000e+00, -9.0928661148196862e-09, 0.0000000000000000e+00, 4.5464330574098431e-09, 0.0000000000000000e+00, -9.0928661148196858e-10},
{9.0928661148203424e-10, 0.0000000000000000e+00, -4.5464330574101715e-09, 0.0000000000000000e+00, 9.0928661148203430e-09, 0.0000000000000000e+00, -9.0928661148203430e-09, 0.0000000000000000e+00, 4.5464330574101715e-09, 0.0000000000000000e+00, -9.0928661148203424e-10},
{9.0928661148203041e-10, 0.0000000000000000e+00, -4.5464330574101516e-09, 0.0000000000000000e+00, 9.0928661148203033e-09, 0.0000000000000000e+00, -9.0928661148203033e-09, 0.0000000000000000e+00, 4.5464330574101516e-09, 0.0000000000000000e+00, -9.0928661148203041e-10},
{9.0928661148164309e-10, 0.0000000000000000e+00, -4.5464330574082152e-09, 0.0000000000000000e+00, 9.0928661148164304e-09, 0.0000000000000000e+00, -9.0928661148164304e-09, 0.0000000000000000e+00, 4.5464330574082152e-09, 0.0000000000000000e+00, -9.0928661148164309e-10},
{9.0928661148193684e-10, 0.0000000000000000e+00, -4.5464330574096843e-09, 0.0000000000000000e+00, 9.0928661148193686e-09, 0.0000000000000000e+00, -9.0928661148193686e-09, 0.0000000000000000e+00, 4.5464330574096843e-09, 0.0000000000000000e+00, -9.0928661148193684e-10},
{9.0928661148192133e-10, 0.0000000000000000e+00, -4.5464330574096065e-09, 0.0000000000000000e+00, 9.0928661148192131e-09, 0.0000000000000000e+00, -9.0928661148192131e-09, 0.0000000000000000e+00, 4.5464330574096065e-09, 0.0000000000000000e+00, -9.0928661148192133e-10},
{9.0928661148181700e-10, 0.0000000000000000e+00, -4.5464330574090846e-09, 0.0000000000000000e+00, 9.0928661148181692e-09, 0.0000000000000000e+00, -9.0928661148181692e-09, 0.0000000000000000e+00, 4.5464330574090846e-09, 0.0000000000000000e+00, -9.0928661148181700e-10},
{9.0928661148189248e-10, 0.0000000000000000e+00, -4.5464330574094626e-09, 0.0000000000000000e+00, 9.0928661148189252e-09, 0.0000000000000000e+00, -9.0928661148189252e-09, 0.0000000000000000e+00, 4.5464330574094626e-09, 0.0000000000000000e+00, -9.0928661148189248e-10}
};


//creating queues
static queue_t zQueue[FILTER_IIR_FILTER_COUNT];	
static queue_t xQueue;	
static queue_t yQueue;	
static queue_t outputQueue[FILTER_IIR_FILTER_COUNT];
static double computePowerValue[FILTER_IIR_FILTER_COUNT];
static double oldestValue[FILTER_IIR_FILTER_COUNT];


 
//intializing zQueues to be filled with zeros
static void initZQueues() {
  for (uint32_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++) {
    queue_init(&(zQueue[i]), Z_QUEUE_SIZE, "zQueue");
    for (uint32_t j = 0; j < Z_QUEUE_SIZE; j++)
     queue_overwritePush(&(zQueue[i]), QUEUE_INIT_VALUE); //filling with zeros
  }
}

//intializing xQueues to be filled with zeros
static void initXQueues() {
    queue_init(&(xQueue), X_QUEUE_SIZE, "xQueue");
    for (uint32_t j = 0; j < X_QUEUE_SIZE; j++)
     queue_overwritePush(&(xQueue), QUEUE_INIT_VALUE); //filling with zeros
  
}

//intializing yQueues to be filled with zeros
static void initYQueues() {
    queue_init(&(yQueue), Y_QUEUE_SIZE, "yQueue");
    for (uint32_t j = 0; j < Y_QUEUE_SIZE; j++)
     queue_overwritePush(&(yQueue), QUEUE_INIT_VALUE); //filling with zeros
  
}

//intializing outputQueues to be filled with zeros
void initOutputQueues() {
  for (uint32_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++) {
    queue_init(&(outputQueue[i]), OUTPUT_QUEUE_SIZE, "outputQueue");
    for (uint32_t j = 0; j < OUTPUT_QUEUE_SIZE; j++)
     queue_overwritePush(&(outputQueue[i]), QUEUE_INIT_VALUE); //filling with zeros
  }
}

//intializing outputQueues to be filled with zeros
void initComputePowerQueues() {
    for (uint32_t j = 0; j < FILTER_IIR_FILTER_COUNT; j++) {
        computePowerValue[j] = QUEUE_INIT_VALUE; //filling with zeros
        oldestValue[j] = QUEUE_INIT_VALUE; //filling with zeros
    }
  
}


/******************************************************************************
***** Main Filter Functions
******************************************************************************/

// Must call this prior to using any filter functions.
void filter_init() {
    //initialize the x, y, z queues for filtering
    initXQueues();
    initYQueues();
    initZQueues();
    
    //initialize our double queues for the filtering
    initComputePowerQueues();
    initOutputQueues();
}

// Use this to copy an input into the input queue of the FIR-filter (xQueue).
void filter_addNewInput(double x) {
    queue_overwritePush(&(xQueue), x);
}

// Invokes the FIR-filter. Input is contents of xQueue.
// Output is returned and is also pushed on to yQueue.
double filter_firFilter() {
    queue_data_t newData;
    double total = 0.0;
    for (uint16_t i = 0; i < FIR_FILTER_TAP_COUNT; i++) {
        total +=  (queue_readElementAt(&(xQueue), (FIR_FILTER_TAP_COUNT - i - 1)) * firCoefficients[i]); //iterate through the xQueue and apply FIR filter
    }
    newData = total;
    queue_overwritePush(&(yQueue), newData);
    return total;
}

// Use this to invoke a single iir filter. Input comes from yQueue.
// Output is returned and is also pushed onto zQueue[filterNumber].
double filter_iirFilter(uint16_t filterNumber) {
    queue_data_t newData;
    double total = 0.0;
    total += iirBCoefficientConstants[filterNumber][IIR_B_COEFFICIENT_COUNT-1] * queue_readElementAt(&(yQueue), 0);
    //iterate through the yQueue and apply iir filter
    for (uint16_t i = 0; i < IIR_A_COEFFICIENT_COUNT; i++) {
        total +=  ((queue_readElementAt(&(yQueue), (IIR_B_COEFFICIENT_COUNT - i - 1)) * iirBCoefficientConstants[filterNumber][i]) - (queue_readElementAt(&(zQueue[filterNumber]), (IIR_A_COEFFICIENT_COUNT - i - 1)) * iirACoefficientConstants[filterNumber][i]));
    }
    newData = total;
    //Overwrite the data for both zQueue and outputQueue
    queue_overwritePush(&(zQueue[filterNumber]), newData);
    queue_overwritePush(&(outputQueue[filterNumber]),newData);
    return total;
}

// Use this to compute the power for values contained in an outputQueue.
// If force == true, then recompute power by using all values in the
// outputQueue. This option is necessary so that you can correctly compute power
// values the first time. After that, you can incrementally compute power values
// by:
// 1. Keeping track of the power computed in a previous run, call this
// prev-power.
// 2. Keeping track of the oldest outputQueue value used in a previous run, call
// this oldestValue.
// 3. Get the newest value from the power queue, call this newest-value.
// 4. Compute new power as: prev-power - (oldestValue * oldestValue) +
// (newest-value * newest-value). Note that this function will probably need an
// array to keep track of these values for each of the 10 output queues.
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch, bool debugPrint) {
    double total = 0.0;
    //If forceComputeFromScratch = true, compute from all values in outputQueue
    if (forceComputeFromScratch) {
        for (uint16_t i = 0; i < OUTPUT_QUEUE_SIZE; i++) {
            total += queue_readElementAt(&outputQueue[filterNumber], i) * queue_readElementAt(&outputQueue[filterNumber], i); //Adding up total when force == 1
        }
        //Adding our total into the filter
        computePowerValue[filterNumber] = total;
        oldestValue[filterNumber] = queue_readElementAt(&outputQueue[filterNumber], 0);
        return total; //Skip the else statement
        }
    //Setting up total if force != 1
    total = computePowerValue[filterNumber] - (oldestValue[filterNumber] * oldestValue[filterNumber]) + (queue_readElementAt(&outputQueue[filterNumber], OUTPUT_QUEUE_SIZE - 1) * queue_readElementAt(&outputQueue[filterNumber], OUTPUT_QUEUE_SIZE - 1));
    oldestValue[filterNumber] = queue_readElementAt(&outputQueue[filterNumber], 0);
    computePowerValue[filterNumber] = total; // Adding total to our array
    return total;
}

// Returns the last-computed output power value for the IIR filter
// [filterNumber].
double filter_getCurrentPowerValue(uint16_t filterNumber) {
    return computePowerValue[filterNumber];
}

// Sets a current power value for a specific filter number.
// Useful in testing the detector.
void filter_setCurrentPowerValue(uint16_t filterNumber, double value) {
    computePowerValue[filterNumber] = value;
}

// Get a copy of the current power values.
// This function copies the already computed values into a previously-declared
// array so that they can be accessed from outside the filter software by the
// detector. Remember that when you pass an array into a C function, changes to
// the array within that function are reflected in the returned array.
void filter_getCurrentPowerValues(double powerValues[]) {
    for (uint16_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++) {
        powerValues[i] = computePowerValue[i];
    }
}

// Using the previously-computed power values that are currently stored in
// currentPowerValue[] array, copy these values into the normalizedArray[]
// argument and then normalize them by dividing all of the values in
// normalizedArray by the maximum power value contained in currentPowerValue[].
// The pointer argument indexOfMaxValue is used to return the index of the
// maximum value. If the maximum power is zero, make sure to not divide by zero
// and that *indexOfMaxValue is initialized to a sane value (like zero).
void filter_getNormalizedPowerValues(double normalizedArray[], uint16_t *indexOfMaxValue) {
    uint16_t maxIndex = 0;
    //Iterate through power values and find max index
    for (uint16_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++) {
        if (computePowerValue[i] > computePowerValue[maxIndex]) {
            maxIndex = i;
        }
    }

    //Copy normalized power values into normalizedArray
    for (uint16_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++) {
        if (computePowerValue[maxIndex] != 0) {
            normalizedArray[i] = computePowerValue[i] / computePowerValue[maxIndex];
        }
        //DONT DIVIDE BY 0
        else {
            normalizedArray[i] = 0;
        }
    }

    *indexOfMaxValue = maxIndex;
}

/******************************************************************************
***** Verification-Assisting Functions
***** External test functions access the internal data structures of filter.c
***** via these functions. They are not used by the main filter functions.
******************************************************************************/

// Returns the array of FIR coefficients.
const double *filter_getFirCoefficientArray() {
    return firCoefficients;
}

// Returns the number of FIR coefficients.
uint32_t filter_getFirCoefficientCount() {
    return FIR_FILTER_COEFFICIENT_COUNT;
}

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirACoefficientArray(uint16_t filterNumber) {
    return iirACoefficientConstants[filterNumber];
}

// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount() {
    return IIR_A_COEFFICIENT_COUNT;
}

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirBCoefficientArray(uint16_t filterNumber) {
    return iirBCoefficientConstants[filterNumber];
}

// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount() {
    return IIR_B_COEFFICIENT_COUNT;
}

// Returns the size of the yQueue.
uint32_t filter_getYQueueSize() {
    return Y_QUEUE_SIZE;
}

// Returns the decimation value.
uint16_t filter_getDecimationValue() {
    return DECIMATION_VALUE;
}

// Returns the address of xQueue.
queue_t *filter_getXQueue() {
    return &(xQueue);
}

// Returns the address of yQueue.
queue_t *filter_getYQueue() {
    return &(yQueue);
}

// Returns the address of zQueue for a specific filter number.
queue_t *filter_getZQueue(uint16_t filterNumber) {
    return &zQueue[filterNumber];
}

// Returns the address of the IIR output-queue for a specific filter-number.
queue_t *filter_getIirOutputQueue(uint16_t filterNumber) {
    return &outputQueue[filterNumber];
}
