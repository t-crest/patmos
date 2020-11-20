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

/*
    Title: USBEndpointDescriptor implementation

    About: Purpose
        Implementation of the USBEndpointDescriptor class.
*/

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include "USBEndpointDescriptor.h"

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Returns the number of an endpoint given its descriptor.
/// \param endpoint Pointer to a USBEndpointDescriptor instance.
/// \return Endpoint number.
//------------------------------------------------------------------------------
unsigned char USBEndpointDescriptor_GetNumber(
    const USBEndpointDescriptor *endpoint)
{
    return endpoint->bEndpointAddress & 0xF;
}

//------------------------------------------------------------------------------
/// Returns the direction of an endpoint given its descriptor.
/// \param endpoint Pointer to a USBEndpointDescriptor instance.
/// \return Endpoint direction (see <Endpoint directions>).
//------------------------------------------------------------------------------
unsigned char USBEndpointDescriptor_GetDirection(
    const USBEndpointDescriptor *endpoint)
{
    if ((endpoint->bEndpointAddress & 0x80) != 0) {

        return USBEndpointDescriptor_IN;
    }
    else {

        return USBEndpointDescriptor_OUT;
    }
}

//------------------------------------------------------------------------------
/// Returns the type of an endpoint given its descriptor.
/// \param endpoint Pointer to a USBEndpointDescriptor instance.
/// \return Endpoint type (see <Endpoint types>).
//------------------------------------------------------------------------------
unsigned char USBEndpointDescriptor_GetType(
    const USBEndpointDescriptor *endpoint)
{
    return endpoint->bmAttributes & 0x3;
}

//------------------------------------------------------------------------------
/// Returns the maximum size of a packet (in bytes) on an endpoint given
/// its descriptor.
/// \param endpoint - Pointer to a USBEndpointDescriptor instance.
/// \return Maximum packet size of endpoint.
//------------------------------------------------------------------------------
unsigned short USBEndpointDescriptor_GetMaxPacketSize(
    const USBEndpointDescriptor *endpoint)
{
    return endpoint->wMaxPacketSize;
}

//------------------------------------------------------------------------------
/// Returns the polling interval on an endpoint given its descriptor.
/// \param endpoint - Pointer to a USBEndpointDescriptor instance.
/// \return Polling interval of endpoint.
//------------------------------------------------------------------------------
unsigned char USBEndpointDescriptor_GetInterval(
    const USBEndpointDescriptor *endpoint)
{
    return endpoint->bInterval;
}

