/* $Id: platform_message.h,v 1.6 2010-06-29 22:07:53 scipio Exp $
 * Copyright (c) 2005 The Regents of the University  of California.  
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
 * - Neither the name of the University of California nor the names of
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
 * Copyright (c) 2002-2005 Intel Corporation
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE     
 * file. If you do not find these files, copies can be found by writing to
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300, Berkeley, CA, 
 * 94704.  Attention:  Intel License Inquiry.
 */

/**
 * Defining the platform-independently named packet structures to be the
 * chip-specific TDA5250 packet structures.
 *
 * @author Philip Levis
 * @author Vlado Handziski (TDA5250 Modifications)
 * @date   May 16 2005
 * Revision:  $Revision: 1.6 $
 */


#ifndef PLATFORM_MESSAGE_H
#define PLATFORM_MESSAGE_H

#include "Serial.h"
#include "tda5250_message.h"

#ifndef TOSH_DATA_LENGTH
#define TOSH_DATA_LENGTH 48
#endif

typedef union message_header_t {
  tda5250_header_t radio;
  serial_header_t serial;
} message_header_t;

typedef union message_footer_t {
  tda5250_footer_t radio;
} message_footer_t;

typedef union message_metadata_t {
  tda5250_metadata_t radio;
} message_metadata_t;

typedef tda5250_header_t message_radio_header_t;
typedef tda5250_footer_t message_radio_footer_t;
typedef tda5250_metadata_t message_radio_metadata_t;

#if TOSH_DATA_LENGTH < 33
#error "TOSH_DATA_LENGH must be larger than 33 bytes for this platform."
#endif

#endif
