/*
 * Copyright (c) 1988-2002
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef decrypt_interface_h
#define decrypt_interface_h

typedef struct netdissect_options netdissect_options;

struct netdissect_options {
  char *ndo_espsecret;
  struct sa_list *ndo_sa_list_head;  /* used by print-esp.c */
  struct sa_list *ndo_sa_default;

  /*global pointers to beginning and end of current packet (during printing) */
  const u_char *ndo_packetp;
  const u_char *ndo_snapend;

  char *ip_hdr;
  unsigned int ip_len;
};

typedef u_char cookie_t[8];
typedef u_char msgid_t[4];

#define PORT_ISAKMP 500


/* 3.1 ISAKMP Header Format (IKEv1 and IKEv2)
         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        !                          Initiator                            !
        !                            Cookie                             !
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        !                          Responder                            !
        !                            Cookie                             !
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        !  Next Payload ! MjVer ! MnVer ! Exchange Type !     Flags     !
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        !                          Message ID                           !
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        !                            Length                             !
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
struct isakmp {
    cookie_t i_ck;      /* Initiator Cookie */
    cookie_t r_ck;      /* Responder Cookie */
    uint8_t np;     /* Next Payload Type */
    uint8_t vers;
#define ISAKMP_VERS_MAJOR   0xf0
#define ISAKMP_VERS_MAJOR_SHIFT 4
#define ISAKMP_VERS_MINOR   0x0f
#define ISAKMP_VERS_MINOR_SHIFT 0
    uint8_t etype;      /* Exchange Type */
    uint8_t flags;      /* Flags */
    msgid_t msgid;
    uint32_t len;       /* Length */
};

/* 3.2 Payload Generic Header
         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        ! Next Payload  !   RESERVED    !         Payload Length        !
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
struct isakmp_gen {
    uint8_t  np;       /* Next Payload */
    uint8_t  critical; /* bit 7 - critical, rest is RESERVED */
    uint16_t len;      /* Payload Length */
};


#define IKEv1_MAJOR_VERSION  1
#define IKEv1_MINOR_VERSION  0

#define IKEv2_MAJOR_VERSION  2
#define IKEv2_MINOR_VERSION  0

/* Flags */
#define ISAKMP_FLAG_E 0x01 /* Encryption Bit */
#define ISAKMP_FLAG_C 0x02 /* Commit Bit */
#define ISAKMP_FLAG_extra 0x04

/* IKEv2 */
#define ISAKMP_FLAG_I (1 << 3)  /* (I)nitiator */
#define ISAKMP_FLAG_V (1 << 4)  /* (V)ersion   */
#define ISAKMP_FLAG_R (1 << 5)  /* (R)esponse  */

#define ISAKMP_NPTYPE_v2E 46 /* v2 Encrypted payload */

struct newesp {
    uint32_t    esp_spi;    /* ESP */
    uint32_t    esp_seq;    /* Sequence number */
    /*variable size*/       /* (IV and) Payload data */
    /*variable size*/       /* padding */
    /*8bit*/            /* pad size */
    /*8bit*/            /* next header */
    /*8bit*/            /* next header */
    /*variable size, 32bit bound*/  /* Authentication data */
};

#define UNALIGNED_MEMCPY(p, q, l)   memcpy((p), (q), (l))
#define UNALIGNED_MEMCMP(p, q, l)   memcmp((p), (q), (l))

extern int
mdmi_decrypt(netdissect_options *ndo, const u_char *bp, unsigned int caplen);

#endif
