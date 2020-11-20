/*
 * Copyright (c) 2000-2004 The Regents of the University  of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the copyright holders nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright (c) 2007 Johns Hopkins University.
 * All rights reserved.
 */

/**
 * @author Jonathan Hui <jwhui@cs.berkeley.edu>
 * @author Chieh-Jan Mike Liang <cliang4@cs.jhu.edu>
 * @author Razvan Musaloiu-E. <razvanm@cs.jhu.edu>
 */

#ifndef DELUGEPAGETRANSFER_H
#define DELUGEPAGETRANSFER_H

#if defined(PLATFORM_TELOSB) || defined(PLATFORM_EPIC)
  #include "extra/telosb/TOSBoot_platform.h"
#elif defined(PLATFORM_MICAZ) || defined(PLATFORM_IRIS)
  #include "extra/micaz/TOSBoot_platform.h"
#elif defined(PLATFORM_MULLE)
  #include "extra/mulle/TOSBoot_platform.h"
#elif defined(PLATFORM_TINYNODE)
  #include "extra/tinynode/TOSBoot_platform.h"
#else
  #error "Target platform is not currently supported by Deluge T2"
#endif

#include <message.h>

#define AM_DELUGEADVMSG  0x50
#define AM_DELUGEREQMSG  0x51
#define AM_DELUGEDATAMSG 0x52

typedef int32_t object_id_t;
typedef nx_int32_t nx_object_id_t;
typedef uint32_t object_size_t;
typedef nx_uint32_t nx_object_size_t;
typedef uint8_t page_num_t;
typedef nx_uint8_t nx_page_num_t;

enum {
  DELUGET2_PKT_PAYLOAD_SIZE  = TOSH_DATA_LENGTH - sizeof(nx_object_id_t) - sizeof(nx_page_num_t) - sizeof(nx_uint8_t),
  DELUGET2_BYTES_PER_PAGE    = 1024,
  DELUGET2_PKTS_PER_PAGE     = ((DELUGET2_BYTES_PER_PAGE - 1) / DELUGET2_PKT_PAYLOAD_SIZE) + 1,
  DELUGET2_PKT_BITVEC_SIZE   = (((DELUGET2_PKTS_PER_PAGE - 1) / 8) + 1),

  DELUGE_PKT_PAYLOAD_SIZE           = 23,
  DELUGE_PKTS_PER_PAGE              = 48,
  DELUGE_BYTES_PER_PAGE             = (DELUGE_PKTS_PER_PAGE*DELUGE_PKT_PAYLOAD_SIZE),

  DELUGE_VERSION                    = 2,
  DELUGE_MAX_ADV_PERIOD_LOG2        = 22,
  DELUGE_NUM_NEWDATA_ADVS_REQUIRED  = 2,
  DELUGE_NUM_MIN_ADV_PERIODS        = 2,
  DELUGE_MAX_NUM_REQ_TRIES          = 1,
  DELUGE_REBOOT_DELAY               = 4,
  DELUGE_FAILED_SEND_DELAY          = 16,
  DELUGE_MIN_DELAY                  = 16,
//  DELUGE_DATA_OFFSET                = 128,
  DELUGE_IDENT_SIZE                 = 128,
  DELUGE_INVALID_ADDR               = (0x7fffffffL),
  DELUGE_MIN_ADV_PERIOD_LOG2        = 9,
  DELUGE_MAX_REQ_DELAY              = (0x1L << (DELUGE_MIN_ADV_PERIOD_LOG2 - 1)),
  DELUGE_NACK_TIMEOUT               = (DELUGE_MAX_REQ_DELAY >> 0x1),
  DELUGE_MAX_IMAGE_SIZE             = (128L * 1024L),
  DELUGE_MAX_PAGES                  = 128,
  DELUGE_CRC_SIZE                   = sizeof(uint16_t),
  DELUGE_CRC_BLOCK_SIZE             = DELUGE_MAX_PAGES * DELUGE_CRC_SIZE,
  DELUGE_GOLDEN_IMAGE_NUM           = 0x0,
  DELUGE_INVALID_OBJID              = 0xff,
  DELUGE_INVALID_PKTNUM             = 0xff,
  DELUGE_INVALID_PGNUM              = 0xff,
  DELUGE_QSIZE                      = 2
};

typedef struct DelugeAdvTimer {
  uint32_t timer      : 32;
  uint8_t  periodLog2 : 8;
  bool     overheard  : 1;
  uint8_t  newAdvs    : 7;
} DelugeAdvTimer;

typedef nx_struct DelugeObjDesc {
  nx_object_id_t objid;
  nx_page_num_t  numPgs;         // num pages of complete image
  nx_uint16_t    crc;            // crc for vNum and numPgs
  nx_page_num_t  numPgsComplete; // numPgsComplete in image
  nx_uint8_t     reserved;
} DelugeObjDesc;

#endif
