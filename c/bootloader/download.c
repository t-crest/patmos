/*
   Copyright 2014 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * Download application through the serial line
 * 
 * Authors: Tórur Biskopstø Strøm (torur.strom@gmail.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

#include "boot.h"
#include "include/patio.h"

#define COMPRESSION

#ifdef COMPRESSION
void decompress_init(void);
int decompress_get_byte(void);
#endif

int uart_read(void) {
  while ((UART_STATUS & 0x02) == 0) {
    /* wait for data */
  }
  return UART_DATA;
}

static int get_byte() {
#ifdef COMPRESSION
  return decompress_get_byte();
#else
  return uart_read();
#endif
}

entrypoint_t download(void) {

  unsigned int entrypoint = 0;
  unsigned int section_number = -1;
  unsigned int section_count = 0;
  unsigned int section_filesize = 0;
  unsigned int section_offset = 0;
  unsigned int section_memsize = 0;
  unsigned int section_byte_count = 0;
  unsigned int integer = 0;
  enum state { STATE_ENTRYPOINT, STATE_SECTION_NUMBER,
               STATE_SECTION_FILESIZE, STATE_SECTION_OFFSET, STATE_SECTION_MEMSIZE,
               STATE_SECTION_DATA };

  enum state current_state = STATE_ENTRYPOINT;

  //Packet stuff
  unsigned int CRC_LENGTH = 4;
  unsigned int packet_byte_count = 0;
  unsigned int packet_size = 0;
  unsigned int calculated_crc = 0;
  unsigned int received_crc = 0xFFFFFFFF; //Flipped initial value
  unsigned int poly = 0xEDB88320; //Reversed polynomial

#ifdef COMPRESSION
  decompress_init();
#endif

  for (;;) {
    LEDS = current_state;
    int data = get_byte();

    if (packet_size == 0) {
      //First received byte sets the packet size
      packet_size = data;
      packet_byte_count = 0;
      calculated_crc = 0xFFFFFFFF;
      received_crc = 0;

    } else {
      if (packet_byte_count < CRC_LENGTH) {
        received_crc <<= 8;
        received_crc |= data;
      } else if (packet_byte_count < packet_size+CRC_LENGTH) {
        calculated_crc = calculated_crc ^ data;
        for (int i = 0; i < 8; ++i) {
          if ((calculated_crc & 1) > 0) {
            calculated_crc = (calculated_crc >> 1) ^ poly;
          } else {
            calculated_crc = (calculated_crc >> 1);
          }
        }

        integer |= data << ((3-(section_byte_count%4))*8);
        section_byte_count++;

        if (current_state < STATE_SECTION_DATA) {
          if (section_byte_count == 4) {
            switch(current_state) {
            case STATE_ENTRYPOINT:
              entrypoint = integer;
              break;
            case STATE_SECTION_NUMBER:
              section_number = integer;
              break;
            case STATE_SECTION_FILESIZE:
              section_filesize = integer;
              break;
            case STATE_SECTION_OFFSET:
              section_offset = integer;
              break;
            case STATE_SECTION_MEMSIZE:
              section_memsize = integer;
              break;
            default:
              /* never happens */;
            }

            section_byte_count = 0;
            current_state++;
          }
        } else {
          //In case of data less than 4 bytes write everytime
          //Write to ISPM
          if ((section_offset+section_byte_count-1) >> 16 == 0x01) {
            if (((section_offset+section_byte_count-1) & 0x0000FFFF) < get_ispm_size() ) {
              // With in the ISPM
              *(SPM+(section_offset+section_byte_count-1)/4) = integer;
            } else {
              //Not within ISPM
              //TODO: create warning
            }
          }
          //Write to main memory
          *(MEM+(section_offset+section_byte_count-1)/4) = integer;
        }

        if (current_state == STATE_SECTION_DATA &&
            section_byte_count == section_filesize) {
          // Align to next word boundary
          section_byte_count = (section_byte_count + 3) & ~3;
          // Fill up uninitialized areas with zeros
          while (section_byte_count < section_memsize) {
            if ((section_offset+section_byte_count) >> 16 == 0x01) {
              *(SPM+(section_offset+section_byte_count)/4) = 0;
            }
            *(MEM+(section_offset+section_byte_count)/4) = 0;
            section_byte_count += 4;
          }
          // Values for next segment
          section_byte_count = 0;
          section_count++;
          current_state = STATE_SECTION_FILESIZE;
        }

        if (section_byte_count%4 == 0) {
          integer = 0;
        }
      }

      packet_byte_count++;
      if (packet_byte_count == packet_size+CRC_LENGTH) {
        calculated_crc = calculated_crc ^ 0xFFFFFFFF; //Flipped final value

        UART_DATA = calculated_crc;

        if (calculated_crc != received_crc) {
          return NULL;
        }
        if (section_count == section_number) {
          //End of program transmission
          return (volatile int (*)())entrypoint;
        }

        packet_size = 0;
      }
    }
  }
}
