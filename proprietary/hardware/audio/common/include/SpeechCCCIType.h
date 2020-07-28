#ifndef SPEECH_CCCI_TYPE_H
#define SPEECH_CCCI_TYPE_H

#include <stdint.h>
#include <system/audio.h>

namespace android {

/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define CCCI_MAX_BUF_SIZE               (3456)

#define CCCI_MAILBOX_SIZE               (16) /* sizeof(ccci_mail_box_t) */
#define CCCI_MAX_AP_PAYLOAD_HEADER_SIZE (6)
#define CCCI_MAX_MD_PAYLOAD_HEADER_SIZE (10)
#define CCCI_MAX_AP_PAYLOAD_DATA_SIZE   (CCCI_MAX_BUF_SIZE - CCCI_MAILBOX_SIZE - CCCI_MAX_AP_PAYLOAD_HEADER_SIZE)
#define CCCI_MAX_MD_PAYLOAD_DATA_SIZE   (CCCI_MAX_BUF_SIZE - CCCI_MAILBOX_SIZE - CCCI_MAX_MD_PAYLOAD_HEADER_SIZE)

#define CCCI_MAILBOX_MAGIC              (0xFFFFFFFF)

#define CCCI_AP_PAYLOAD_SYNC            (0xA2A2)
#define CCCI_MD_PAYLOAD_SYNC            (0x1234) /* 0x2A2A is old lagecy code */


#define MAX_SIZE_OF_ONE_FRAME           (16) /* 32-bits * 4ch */

}
#endif // end of SPEECH_CCCI_TYPE_H
