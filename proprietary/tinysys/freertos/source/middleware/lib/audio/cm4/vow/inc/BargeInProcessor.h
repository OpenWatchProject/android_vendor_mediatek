#ifndef BARGEINPROCCESSOR2_H
#define BARGEINPROCCESSOR2_H


#ifdef  __cplusplus
extern  "C" {
#endif

const char* AEC_version();
    /* get current AEC version, in string style
     */



int AEC_init(const int micNumber,
              const int refNumber,
              const char* micConfPath,
              const int processSampleNumber, const int inputUplinkSampleDelay);
    /* init the memory of AEC
     * [micNumber](currently, only "1 and 2" is accepted)
     * [refNumber](currently, only "1" is accepted)
     * [micConfPath] the configuration file path indicates the microphone configuration,
     *               including positions and optimization parameters;
     *               note that the configuration file is generated based on customer's mic coordinates and
     *               provided by algorithm developer
     * [processSampleNumber] indicate the sample number of "one" channel while invoking each process
     * [uplinkSampleDelay] uplink delay downlink sample number (input "-1" use default sample delay)
     * return 1 as success, else as failure
     */

void AEC_release();
    /* release memory of AEC*/

int AEC_writeMicInput(int micIndex, const void* micInputSignals, const int micInputSignalsByteNumber);
    /* must be called for each mic before AEC_process;
     * [micIndex] Index of mic input(currently, only "0 and 1" is accepted)
     * [micInputSignals] pointer to the buffer of input signals;
     *               each sample is considered as 16bit integer (signed)
     * [micInputSignalsByteNumber] total byte number of inputSignals;
     *               must be exactly equal to sizeof(short)*processSampleNumber
     * return 1 as success, else as failure
     */

int AEC_writeRefInput(int refIndex, const void* refInputSignals, const int refInputSignalsByteNumber);
    /* must be called for each ref before AEC_process;
     * [refIndex] Index of echo ref(currently, only "0" is accepted)
     * [refInputSignals] pointer to the buffer of input signals;
     *               each sample is considered as 16bit integer (signed)
     * [refInputSignalsByteNumber] total byte number of inputSignals;
     *               must be exactly equal to sizeof(short)*processSampleNumber
     * return 1 as success, else as failure
     */

int AEC_process();
    /* process the signals
     * return 1 as success, else as failure
     */

int AEC_readMicOutput(int micIndex, void* outputSignals, int* outputSignalsByteNumber);

    /* read processed mic output
     * [micIndex] Index of mic output(currently, only "0 and 1" is accepted)
     * [outputSignals] buffer pointer for writing result of the signal process;
     *               each sample is considered as 16bit integer (signed);
     * [outputSignalsByteNumber] pointer to retrieve the byte number written in the outputSignals
     *               use <0 value to indicate invalid outputSignals result
     * return 1 as success, else as failure
     */



int AEC_getProcessOutputByteNumber();
    /* get outputSignalsByteNumber of a successful AEC_process invoking;
     * this is a convenient interface for upper-level framework to alloc outputSignals buffer
     */



#ifdef  __cplusplus
}
#endif
#endif // BARGEINPROCCESSOR2_H


