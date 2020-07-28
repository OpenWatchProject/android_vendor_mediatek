#ifndef _RDA_EXPORT_H_
#define _RDA_EXPORT_H_

extern int rda_check(unsigned int options, unsigned int enabled_root_symptom_1, 
    unsigned int enabled_root_symptom_2, 
    unsigned int expected_root_count_threshold,
    unsigned int max_timeout );
extern int get_root_status(unsigned int *root_status, unsigned int *root_status1);


#define OPT_UPDATE_ROOT_BASED_ON_UNION          (1<<1)
#define OPT_UPDATE_ROOT_IN_A_TIMELY_FASHION     (1<<2)

/* bit mask 1 for enabling root symptom detection, starting from bit 1,
 * bit 0 is for sleep timeout, in case normal world rda responds > 1.3 * timeout
 */
#define SEC_MOUNT_ROOTFS_RW                 (1U<<1)
#define SEC_MOUNT_SYSTEM_RW                 (1U<<2)
#define SEC_FOUND_ADBD_AS_ROOT              (1U<<3)    //adbd uid is 0(root)
#define SEC_FOUND_PROP_KER_QEMU             (1U<<4)    //ro.kernel.qemu == 1
#define SEC_FOUND_PROP_SECURE               (1U<<5)    //ro.secure == 0
#define SEC_FOUND_PROP_DEBBUGGABLE          (1U<<6)    //ro.debuggable == 1
#define SEC_FOUND_PROP_ADB_ROOT             (1U<<7)    //service.adb.root == 1
#define SEC_FOUND_SU_BINARY                 (1U<<8)    //found su binary under system folders
#define SEC_EXEC_SU_SUCCEED                 (1U<<9)    //be able to execute su using shell
#define SEC_DATA_PERM_CHANGED               (1U<<10)   //be able to write /data using shell or perm is changed
#define SEC_SYSTEM_PERM_CHANGED             (1U<<11)   //be able to write /system using shell or perm is changed
#define SEC_SYSTEM_BIN_PERM_CHANGED         (1U<<12)   //be able to write /system/bin using shell or perm is changed
#define SEC_SYSTEM_XBIN_PERM_CHANGED        (1U<<13)   //be able to write /system/xbin using shell or perm is changed
#define SEC_ROOT_PERM_CHANGED               (1U<<14)   //be able to write / using shell  or perm is changed
#define SEC_SBIN_PERM_CHANGED               (1U<<15)   //be able to write /sbin using shell or perm is changed
#define SEC_FOUND_BUSYBOX_BINARY            (1U<<16)   //found busybox binary
#define SEC_ROOT_TRACE_GT_THRESHOLD         (1U<<17)   //found root_trace count > threshold (0 by default)

#define ROOT_SYMPTOM_MAX                    (18)      //current max detectable symptom +1

#define MAX_TIMEOUT     (10*60)  //seconds, no longer than 1 hour(3600) hex us: 0xD693A400



/* bit mask 2 is reserved for future use */


#endif
