/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support 
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the copyright holders nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/**
 \unit

 !!!Purpose

    USB Device Driver class definition.

 !!!Usage

    -# Instanciate a USBDDriver object and initialize it using
       USBDDriver_Initialize.
    -# When a USB SETUP request is received, forward it to the standard
       driver using USBDDriver_RequestHandler.
    -# Check the Remote Wakeup setting via USBDDriver_IsRemoteWakeUpEnabled.
*/

#ifndef USBDDRIVER_H
#define USBDDRIVER_H

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include "USBDDriverDescriptors.h"
#include <usb/common/core/USBGenericRequest.h>

//------------------------------------------------------------------------------
//         Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// USB device driver structure, holding a list of descriptors identifying
/// the device as well as the driver current state.
//------------------------------------------------------------------------------
typedef struct {

    /// List of descriptors used by the device.
    const USBDDriverDescriptors *pDescriptors;
    /// Current setting for each interface.
    unsigned char *pInterfaces;
    /// Current configuration number (0 -> device is not configured).
    unsigned char cfgnum;
    /// Indicates if remote wake up has been enabled by the host.
    unsigned char isRemoteWakeUpEnabled;
#if defined(BOARD_USB_OTGHS)
    /// Features supported by OTG
    unsigned char otg_features_supported;
#endif
} USBDDriver;

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------

extern void USBDDriver_Initialize(
    USBDDriver *pDriver,
    const USBDDriverDescriptors *pDescriptors,
    unsigned char *pInterfaces);

extern void USBDDriver_RequestHandler(
    USBDDriver *pDriver,
    const USBGenericRequest *pRequest);

extern unsigned char USBDDriver_IsRemoteWakeUpEnabled(const USBDDriver *pDriver);

#if defined(BOARD_USB_OTGHS)
extern unsigned char USBDDriver_returnOTGFeatures(void);
extern void USBDDriver_clearOTGFeatures(void);
#endif

#endif //#ifndef USBDDRIVER_H

