#ifndef HWC_HWC2_DEFS_H
#define HWC_HWC2_DEFS_H

#define HWC_NO_ION_FD ((int)(~0U>>1))

typedef enum {
    HWC_SKIP_VALIDATE_NOT_SKIP = 0,
    HWC_SKIP_VALIDATE_SKIP = 1
} SKIP_VALI_STATE;

// Track the sequence state from validate to present
// +-----------------+------------------------------------+----------------------------+
// | State           | Define                             | Next State                 |
// +-----------------+------------------------------------+----------------------------+
// | PRESENT_DONE    | SF get release fence / Initinal    | CHECK_SKIP_VALI / VALIDATE |
// | CHECK_SKIP_VALI | Check or Check done skip validate  | VALIDATE_DONE / VALIDATE   |
// | VALIDATE        | Doing or done validate             | VALIDATE_DONE]             |
// | VALIDATE_DONE   | SF get validate result             | PRESENT                    |
// | PRESENT         | Doing or done validate             | PRESENT_DONE               |
// +-----------------+------------------------------------+----------------------------+

typedef enum {
    HWC_VALI_PRESENT_STATE_PRESENT_DONE = 0,
    HWC_VALI_PRESENT_STATE_CHECK_SKIP_VALI = 1,
    HWC_VALI_PRESENT_STATE_VALIDATE = 2,
    HWC_VALI_PRESENT_STATE_VALIDATE_DONE = 3,
    HWC_VALI_PRESENT_STATE_PRESENT = 4
} HWC_VALI_PRESENT_STATE;

const char* getPresentValiStateString(const HWC_VALI_PRESENT_STATE& state);

typedef enum {
    HWC_COMP_FILE_UNK = 0,
    HWC_COMP_FILE_NSET,
    HWC_COMP_FILE_HWC,
    HWC_COMP_FILE_HWCD,
    HWC_COMP_FILE_HWCL,
    HWC_COMP_FILE_PF,
    HWC_COMP_FILE_PFC,
} HWC_COMP_FILE;

const char* getCompFileString(const HWC_COMP_FILE& comp_file);

#endif
