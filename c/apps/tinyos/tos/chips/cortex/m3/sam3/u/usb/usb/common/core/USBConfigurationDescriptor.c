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
    Title: USBConfigurationDescriptor implementation

    About: Purpose
        Implementation of the USBConfigurationDescriptor class.
*/

//-----------------------------------------------------------------------------
//         Headers
//-----------------------------------------------------------------------------

#include "USBConfigurationDescriptor.h"

//-----------------------------------------------------------------------------
//         Exported functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Returns the total length of a configuration, i.e. including the 
/// descriptors following it.
/// \param configuration Pointer to a USBConfigurationDescriptor instance.
/// \return Total length (in bytes) of the configuration.
//-----------------------------------------------------------------------------
unsigned int USBConfigurationDescriptor_GetTotalLength(
    const USBConfigurationDescriptor *config)
{
    return config->wTotalLength;
}

//-----------------------------------------------------------------------------
/// Returns the number of interfaces in a configuration.
/// \param configuration Pointer to a USBConfigurationDescriptor instance.
/// \return Number of interfaces in configuration.
//-----------------------------------------------------------------------------
unsigned char USBConfigurationDescriptor_GetNumInterfaces(
    const USBConfigurationDescriptor *config)
{
    return config->bNumInterfaces;
}

//-----------------------------------------------------------------------------
/// Indicates if the device is self-powered when in a given configuration.
/// \param configuration Pointer to a USBConfigurationDescriptor instance.
/// \return 1 if the device is self-powered when in the given configuration;
///         otherwise 0.
//-----------------------------------------------------------------------------
unsigned char USBConfigurationDescriptor_IsSelfPowered(
    const USBConfigurationDescriptor *config)
{
    if ((config->bmAttributes & (1 << 6)) != 0) {

        return 1;
    }
    else {

        return 0;
    }
}

//-----------------------------------------------------------------------------
/// Parses the given Configuration descriptor (followed by relevant
/// interface, endpoint and class-specific descriptors) into three arrays.
/// *Each array must have its size equal or greater to the number of
/// descriptors it stores plus one*. A null-value is inserted after the last
/// descriptor of each type to indicate the array end.
///
/// Note that if the pointer to an array is null (0), nothing is stored in
/// it.
/// \param configuration Pointer to the start of the whole Configuration 
///                      descriptor.
/// \param interfaces    Pointer to the Interface descriptor array.
/// \param epoints     Pointer to the Endpoint descriptor array.
/// \param others        Pointer to the class-specific descriptor array.
//-----------------------------------------------------------------------------
void USBConfigurationDescriptor_Parse(
    const USBConfigurationDescriptor *config,
    USBInterfaceDescriptor **interfaces,
    USBEndpointDescriptor **epoints,
    USBGenericDescriptor **others)
{
    int size;
    USBGenericDescriptor *descriptor;
    // Get size of configuration to parse
    size = USBConfigurationDescriptor_GetTotalLength(config);
    size -= sizeof(USBConfigurationDescriptor);

    // Start parsing descriptors
    descriptor = (USBGenericDescriptor *) config;
    while (size > 0) {

        // Get next descriptor
        descriptor = USBGenericDescriptor_GetNextDescriptor(descriptor);
        size -= USBGenericDescriptor_GetLength(descriptor);

        // Store descriptor in correponding array
        if (USBGenericDescriptor_GetType(descriptor)
             == USBGenericDescriptor_INTERFACE) {

            if (interfaces) {
            
                *interfaces = (USBInterfaceDescriptor *) descriptor;
                interfaces++;
            }
        }
        else if (USBGenericDescriptor_GetType(descriptor)
                  == USBGenericDescriptor_ENDPOINT) {

            if (epoints) {
                
                *epoints = (USBEndpointDescriptor *) descriptor;
                epoints++;
            }
        }
        else if (others) {

            *others = descriptor;
            others++;
        }
    }

    // Null-terminate arrays
    if (interfaces) {

        *interfaces = 0;
    }
    if (epoints) {

        *epoints = 0;
    }
    if (others) {

        *others = 0;
    }
}

