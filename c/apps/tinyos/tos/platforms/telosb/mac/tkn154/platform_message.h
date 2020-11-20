
#ifndef PLATFORM_MESSAGE_H
#define PLATFORM_MESSAGE_H

#include <Serial.h>

/* The following include pulls in the ieee154_header_t/ieee154_metadata_t definitions */
#include <TKN154_MAC.h>

/* TOSH_DATA_LENGTH should be the maximum length of the MAC payload */
#ifndef TOSH_DATA_LENGTH
#define TOSH_DATA_LENGTH 118
#elif TOSH_DATA_LENGTH < 118
#warning "MAC payload region is smaller than aMaxMACPayloadSize!"
#endif

typedef union message_header {
  ieee154_header_t ieee154;
  serial_header_t serial;
} message_header_t;

typedef union TOSRadioFooter {
} message_footer_t;

typedef union TOSRadioMetadata {
  ieee154_metadata_t ieee154;
} message_metadata_t;

#endif
