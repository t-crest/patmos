/*
 * -------------------------------------------
 *    MSP432 DriverLib - v3_21_00_05
 * Modified
 * -------------------------------------------
 *
 * --COPYRIGHT--,BSD,BSD
 * Copyright (c) 2016, Texas Instruments Incorporated
 * Copyright (c) 2016, Eric B. Decker
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

#ifndef __ROM_H__
#define __ROM_H__

#ifdef __MSP432_DVRLIB_ROM__

//*****************************************************************************
//
// Pointers to the main API tables.
//
//*****************************************************************************
#define ROM_APITABLE            ((unsigned long *)0x02000800)
#define ROM_VERSION             (ROM_APITABLE[0])
#define ROM_ADC14TABLE          ((unsigned long *)(ROM_APITABLE[1]))
#define ROM_AES256TABLE         ((unsigned long *)(ROM_APITABLE[2]))
#define ROM_COMPTABLE           ((unsigned long *)(ROM_APITABLE[3]))
#define ROM_CRC32TABLE          ((unsigned long *)(ROM_APITABLE[4]))
#define ROM_CSTABLE             ((unsigned long *)(ROM_APITABLE[5]))
#define ROM_DMATABLE            ((unsigned long *)(ROM_APITABLE[6]))
#define ROM_FLASHCTLTABLE       ((unsigned long *)(ROM_APITABLE[7]))
#define ROM_FPUTABLE            ((unsigned long *)(ROM_APITABLE[8]))
#define ROM_GPIOTABLE           ((unsigned long *)(ROM_APITABLE[9]))
#define ROM_I2CTABLE            ((unsigned long *)(ROM_APITABLE[10]))
#define ROM_INTTABLE            ((unsigned long *)(ROM_APITABLE[11]))
#define ROM_MPUTABLE            ((unsigned long *)(ROM_APITABLE[12]))
#define ROM_PCMTABLE            ((unsigned long *)(ROM_APITABLE[13]))
#define ROM_PMAPTABLE           ((unsigned long *)(ROM_APITABLE[14]))
#define ROM_PSSTABLE            ((unsigned long *)(ROM_APITABLE[15]))
#define ROM_REFTABLE            ((unsigned long *)(ROM_APITABLE[16]))
#define ROM_RESETCTLTABLE       ((unsigned long *)(ROM_APITABLE[17]))
#define ROM_RTCTABLE            ((unsigned long *)(ROM_APITABLE[18]))
#define ROM_SPITABLE            ((unsigned long *)(ROM_APITABLE[19]))
#define ROM_SYSCTLTABLE         ((unsigned long *)(ROM_APITABLE[20]))
#define ROM_SYSTICKTABLE        ((unsigned long *)(ROM_APITABLE[21]))
#define ROM_TIMER_ATABLE        ((unsigned long *)(ROM_APITABLE[22]))
#define ROM_TIMER32TABLE        ((unsigned long *)(ROM_APITABLE[23]))
#define ROM_UARTTABLE           ((unsigned long *)(ROM_APITABLE[24]))
#define ROM_WDTTABLE            ((unsigned long *)(ROM_APITABLE[25]))

//*****************************************************************************
//
// Macros for calling ROM functions in the ADC14 API.
//
//*****************************************************************************
#define ROM_ADC14_enableModule                                                \
        ((void (*)(void))ROM_ADC14TABLE[0])
#define ROM_ADC14_disableModule                                               \
        ((bool (*)(void))ROM_ADC14TABLE[1])
#define ROM_ADC14_initModule                                                  \
        ((bool (*)(uint32_t clockSource,                                      \
                   uint32_t clockPredivider,                                  \
                   uint32_t clockDivider,                                     \
                   uint32_t internalChannelMask))ROM_ADC14TABLE[2])
#define ROM_ADC14_setResolution                                               \
        ((void (*)(uint32_t resolution))ROM_ADC14TABLE[3])
#define ROM_ADC14_getResolution                                               \
        ((uint_fast32_t (*)(void))ROM_ADC14TABLE[4])
#define ROM_ADC14_setSampleHoldTrigger                                        \
        ((bool (*)(uint32_t source,                                           \
                   bool invertSignal))ROM_ADC14TABLE[5])
#define ROM_ADC14_setSampleHoldTime                                           \
        ((bool (*)(uint32_t firstPulseWidth,                                  \
                   uint32_t secondPulseWidth))ROM_ADC14TABLE[6])
#define ROM_ADC14_configureMultiSequenceMode                                  \
        ((bool (*)(uint32_t memoryStart,                                      \
                   uint32_t memoryEnd,                                        \
                   bool repeatMode))ROM_ADC14TABLE[7])
#define ROM_ADC14_configureSingleSampleMode                                   \
        ((bool (*)(uint32_t memoryDestination,                                \
                   bool repeatMode))ROM_ADC14TABLE[8])
#define ROM_ADC14_enableConversion                                            \
        ((bool (*)(void))ROM_ADC14TABLE[9])
#define ROM_ADC14_disableConversion                                           \
        ((void (*)(void))ROM_ADC14TABLE[10])
#define ROM_ADC14_isBusy                                                      \
        ((bool (*)(void))ROM_ADC14TABLE[11])
#define ROM_ADC14_configureConversionMemory                                   \
        ((bool (*)(uint32_t memorySelect,                                     \
                   uint32_t refSelect,                                        \
                   uint32_t channelSelect,                                    \
                   bool differntialMode))ROM_ADC14TABLE[12])
#define ROM_ADC14_enableComparatorWindow                                      \
        ((bool (*)(uint32_t memorySelect,                                     \
                   uint32_t windowSelect))ROM_ADC14TABLE[13])
#define ROM_ADC14_disableComparatorWindow                                     \
        ((bool (*)(uint32_t memorySelect))ROM_ADC14TABLE[14])
#define ROM_ADC14_setComparatorWindowValue                                    \
        ((bool (*)(uint32_t window,                                           \
                   int16_t low,                                               \
                   int16_t high))ROM_ADC14TABLE[15])
#define ROM_ADC14_setResultFormat                                             \
        ((bool (*)(uint32_t resultFormat))ROM_ADC14TABLE[16])
#define ROM_ADC14_getResult                                                   \
        ((uint_fast16_t (*)(uint32_t memorySelect))ROM_ADC14TABLE[17])
#define ROM_ADC14_getResultArray                                              \
        ((void (*)(uint32_t memoryStart,                                      \
                   uint32_t memoryEnd,                                        \
                   uint16_t* res))ROM_ADC14TABLE[19])
#define ROM_ADC14_enableReferenceBurst                                        \
        ((bool (*)(void))ROM_ADC14TABLE[20])
#define ROM_ADC14_disableReferenceBurst                                       \
        ((bool (*)(void))ROM_ADC14TABLE[21])
#define ROM_ADC14_setPowerMode                                                \
        ((bool (*)(uint32_t powerMode))ROM_ADC14TABLE[22])
#define ROM_ADC14_enableInterrupt                                             \
        ((void (*)(uint_fast64_t mask))ROM_ADC14TABLE[23])
#define ROM_ADC14_disableInterrupt                                            \
        ((void (*)(uint_fast64_t mask))ROM_ADC14TABLE[24])
#define ROM_ADC14_getInterruptStatus                                          \
        ((uint_fast64_t (*)(void))ROM_ADC14TABLE[25])
#define ROM_ADC14_getEnabledInterruptStatus                                   \
        ((uint_fast64_t (*)(void))ROM_ADC14TABLE[26])
#define ROM_ADC14_clearInterruptFlag                                          \
        ((void (*)(uint_fast64_t mask))ROM_ADC14TABLE[27])
#define ROM_ADC14_toggleConversionTrigger                                     \
        ((bool (*)(void))ROM_ADC14TABLE[28])
#define ROM_ADC14_enableSampleTimer                                           \
        ((bool (*)(uint32_t multiSampleConvert))ROM_ADC14TABLE[29])
#define ROM_ADC14_disableSampleTimer                                          \
        ((bool (*)(void))ROM_ADC14TABLE[30])

//*****************************************************************************
//
// Macros for calling ROM functions in the AES256 API.
//
//*****************************************************************************
#define ROM_AES256_setCipherKey                                               \
        ((bool (*)(uint32_t moduleInstance,                                   \
                   const uint8_t *cipherKey,                                  \
                   uint_fast16_t keyLength))ROM_AES256TABLE[0])
#define ROM_AES256_encryptData                                                \
        ((void (*)(uint32_t moduleInstance,                                   \
                   const uint8_t *data,                                       \
                   uint8_t *encryptedData))ROM_AES256TABLE[1])
#define ROM_AES256_decryptData                                                \
        ((void (*)(uint32_t moduleInstance,                                   \
                   const uint8_t *data,                                       \
                   uint8_t *decryptedData))ROM_AES256TABLE[2])
#define ROM_AES256_setDecipherKey                                             \
        ((bool (*)(uint32_t moduleInstance,                                   \
                   const uint8_t *cipherKey,                                  \
                   uint_fast16_t keyLength))ROM_AES256TABLE[3])
#define ROM_AES256_reset                                                      \
        ((void (*)(uint32_t moduleInstance))ROM_AES256TABLE[4])
#define ROM_AES256_startEncryptData                                           \
        ((void (*)(uint32_t moduleInstance,                                   \
                   const uint8_t *data))ROM_AES256TABLE[5])
#define ROM_AES256_startDecryptData                                           \
        ((void (*)(uint32_t moduleInstance,                                   \
                   const uint8_t *data))ROM_AES256TABLE[6])
#define ROM_AES256_startSetDecipherKey                                        \
        ((bool (*)(uint32_t moduleInstance,                                   \
                   const uint8_t *cipherKey,                                  \
                   uint_fast16_t keyLength))ROM_AES256TABLE[7])
#define ROM_AES256_getDataOut                                                 \
        ((bool (*)(uint32_t moduleInstance,                                   \
                   uint8_t *outputData))ROM_AES256TABLE[8])
#define ROM_AES256_isBusy                                                     \
        ((bool (*)(uint32_t moduleInstance))ROM_AES256TABLE[9])
#define ROM_AES256_clearErrorFlag                                             \
        ((void (*)(uint32_t moduleInstance))ROM_AES256TABLE[10])
#define ROM_AES256_getErrorFlagStatus                                         \
        ((uint32_t (*)(uint32_t moduleInstance))ROM_AES256TABLE[11])
#define ROM_AES256_clearInterruptFlag                                         \
        ((void (*)(uint32_t moduleInstance))ROM_AES256TABLE[12])
#define ROM_AES256_getInterruptStatus                                         \
        ((uint32_t (*)(uint32_t moduleInstance))ROM_AES256TABLE[13])
#define ROM_AES256_enableInterrupt                                            \
        ((void (*)(uint32_t moduleInstance))ROM_AES256TABLE[14])
#define ROM_AES256_disableInterrupt                                           \
        ((void (*)(uint32_t moduleInstance))ROM_AES256TABLE[15])
#define ROM_AES256_getInterruptFlagStatus                                     \
        ((uint32_t (*)(uint32_t moduleInstance))ROM_AES256TABLE[18])

//*****************************************************************************
//
// Macros for calling ROM functions in the Comp API.
//
//*****************************************************************************
#define ROM_COMP_E_initModule                                                 \
        ((bool (*)(uint32_t comparator,                                       \
                   const COMP_E_Config *config))ROM_COMPTABLE[0])
#define ROM_COMP_E_setReferenceVoltage                                        \
        ((void (*)(uint32_t comparator,                                       \
                   uint_fast16_t supplyVoltageReferenceBase,                  \
                   uint_fast16_t lowerLimitSupplyVoltageFractionOf32,         \
                   uint_fast16_t upperLimitSupplyVoltageFractionOf32))ROM_COMPTABLE[1])
#define ROM_COMP_E_setReferenceAccuracy                                       \
        ((void (*)(uint32_t comparator,                                       \
                   uint_fast16_t referenceAccuracy))ROM_COMPTABLE[2])
#define ROM_COMP_E_setPowerMode                                               \
        ((void (*)(uint32_t comparator,                                       \
                   uint_fast16_t powerMode))ROM_COMPTABLE[3])
#define ROM_COMP_E_enableModule                                               \
        ((void (*)(uint32_t comparator))ROM_COMPTABLE[4])
#define ROM_COMP_E_disableModule                                              \
        ((void (*)(uint32_t comparator))ROM_COMPTABLE[5])
#define ROM_COMP_E_shortInputs                                                \
        ((void (*)(uint32_t comparator))ROM_COMPTABLE[6])
#define ROM_COMP_E_unshortInputs                                              \
        ((void (*)(uint32_t comparator))ROM_COMPTABLE[7])
#define ROM_COMP_E_disableInputBuffer                                         \
        ((void (*)(uint32_t comparator,                                       \
                   uint_fast16_t inputPort))ROM_COMPTABLE[8])
#define ROM_COMP_E_enableInputBuffer                                          \
        ((void (*)(uint32_t comparator,                                       \
                   uint_fast16_t inputPort))ROM_COMPTABLE[9])
#define ROM_COMP_E_swapIO                                                     \
        ((void (*)(uint32_t comparator))ROM_COMPTABLE[10])
#define ROM_COMP_E_outputValue                                                \
        ((uint8_t (*)(uint32_t comparator))ROM_COMPTABLE[11])
#define ROM_COMP_E_enableInterrupt                                            \
        ((void (*)(uint32_t comparator,                                       \
                   uint_fast16_t mask))ROM_COMPTABLE[12])
#define ROM_COMP_E_disableInterrupt                                           \
        ((void (*)(uint32_t comparator,                                       \
                   uint_fast16_t mask))ROM_COMPTABLE[13])
#define ROM_COMP_E_clearInterruptFlag                                         \
        ((void (*)(uint32_t comparator,                                       \
                   uint_fast16_t mask))ROM_COMPTABLE[14])
#define ROM_COMP_E_getInterruptStatus                                         \
        ((uint_fast16_t (*)(uint32_t comparator))ROM_COMPTABLE[15])
#define ROM_COMP_E_getEnabledInterruptStatus                                  \
        ((uint_fast16_t (*)(uint32_t comparator))ROM_COMPTABLE[16])
#define ROM_COMP_E_setInterruptEdgeDirection                                  \
        ((void (*)(uint32_t comparator,                                       \
                   uint_fast8_t edgeDirection))ROM_COMPTABLE[17])
#define ROM_COMP_E_toggleInterruptEdgeDirection                               \
        ((void (*)(uint32_t comparator))ROM_COMPTABLE[18])

//*****************************************************************************
//
// Macros for calling ROM functions in the CRC32 API.
//
//*****************************************************************************
#define ROM_CRC32_setSeed                                                     \
        ((void (*)(uint32_t seed,                                             \
                   uint_fast8_t crcType))ROM_CRC32TABLE[0])
#define ROM_CRC32_set8BitData                                                 \
        ((void (*)(uint8_t dataIn,                                            \
                   uint_fast8_t crcType))ROM_CRC32TABLE[1])
#define ROM_CRC32_set16BitData                                                \
        ((void (*)(uint16_t dataIn,                                           \
                   uint_fast8_t crcType))ROM_CRC32TABLE[2])
#define ROM_CRC32_set32BitData                                                \
        ((void (*)(uint32_t dataIn))ROM_CRC32TABLE[3])
#define ROM_CRC32_set8BitDataReversed                                         \
        ((void (*)(uint8_t dataIn,                                            \
                   uint_fast8_t crcType))ROM_CRC32TABLE[4])
#define ROM_CRC32_set16BitDataReversed                                        \
        ((void (*)(uint16_t dataIn,                                           \
                   uint_fast8_t crcType))ROM_CRC32TABLE[5])
#define ROM_CRC32_set32BitDataReversed                                        \
        ((void (*)(uint32_t dataIn))ROM_CRC32TABLE[6])
#define ROM_CRC32_getResult                                                   \
        ((uint32_t (*)(uint_fast8_t crcType))ROM_CRC32TABLE[7])
#define ROM_CRC32_getResultReversed                                           \
        ((uint32_t (*)(uint_fast8_t crcType))ROM_CRC32TABLE[8])

//*****************************************************************************
//
// Macros for calling ROM functions in the CS API.
//
//*****************************************************************************
#define ROM_CS_initClockSignal                                                \
        ((void (*)(uint32_t selectedClockSignal,                              \
                   uint32_t clockSource,                                      \
                   uint32_t clockSourceDivider))ROM_CSTABLE[0])
#define ROM_CS_setReferenceOscillatorFrequency                                \
        ((void (*)(uint8_t referenceFrequency))ROM_CSTABLE[1])
#define ROM_CS_enableClockRequest                                             \
        ((void (*)(uint32_t selectClock))ROM_CSTABLE[2])
#define ROM_CS_disableClockRequest                                            \
        ((void (*)(uint32_t selectClock))ROM_CSTABLE[3])
#define ROM_CS_setDCOCenteredFrequency                                        \
        ((void (*)(uint32_t dcoFreq))ROM_CSTABLE[4])
#define ROM_CS_tuneDCOFrequency                                               \
        ((void (*)(int16_t tuneParameter))ROM_CSTABLE[5])
#define ROM_CS_enableDCOExternalResistor                                      \
        ((void (*)(void))ROM_CSTABLE[6])
#define ROM_CS_disableDCOExternalResistor                                     \
        ((void (*)(void))ROM_CSTABLE[7])
#define ROM_CS_enableInterrupt                                                \
        ((void (*)(uint32_t flags))ROM_CSTABLE[8])
#define ROM_CS_disableInterrupt                                               \
        ((void (*)(uint32_t flags))ROM_CSTABLE[9])
#define ROM_CS_getEnabledInterruptStatus                                      \
        ((uint32_t (*)(void))ROM_CSTABLE[10])
#define ROM_CS_getInterruptStatus                                             \
        ((uint32_t (*)(void))ROM_CSTABLE[11])
#define ROM_CS_setDCOFrequency                                                \
        ((void (*)(uint32_t dcoFrequency))ROM_CSTABLE[12])
#define ROM_CS_getDCOFrequency                                                \
        ((uint32_t (*)(void))ROM_CSTABLE[13])
#define ROM_CS_enableFaultCounter                                             \
        ((void (*)(uint_fast8_t counterSelect))ROM_CSTABLE[14])
#define ROM_CS_disableFaultCounter                                            \
        ((void (*)(uint_fast8_t counterSelect))ROM_CSTABLE[15])
#define ROM_CS_resetFaultCounter                                              \
        ((void (*)(uint_fast8_t counterSelect))ROM_CSTABLE[16])
#define ROM_CS_startFaultCounter                                              \
        ((void (*)(uint_fast8_t counterSelect,                                \
                   uint_fast8_t countValue))ROM_CSTABLE[17])
#define ROM_CS_clearInterruptFlag                                             \
        ((void (*)(uint32_t flags))ROM_CSTABLE[20])
#define ROM_CS_setDCOExternalResistorCalibration                              \
        ((void (*)(uint_fast8_t uiCalData,                                    \
                   uint_fast8_t freqRange))ROM_CSTABLE[31])

//*****************************************************************************
//
// Macros for calling ROM functions in the DMA API.
//
//*****************************************************************************
#define ROM_DMA_enableModule                                                  \
        ((void (*)(void))ROM_DMATABLE[0])
#define ROM_DMA_disableModule                                                 \
        ((void (*)(void))ROM_DMATABLE[1])
#define ROM_DMA_getErrorStatus                                                \
        ((uint32_t (*)(void))ROM_DMATABLE[2])
#define ROM_DMA_clearErrorStatus                                              \
        ((void (*)(void))ROM_DMATABLE[3])
#define ROM_DMA_enableChannel                                                 \
        ((void (*)(uint32_t channelNum))ROM_DMATABLE[4])
#define ROM_DMA_disableChannel                                                \
        ((void (*)(uint32_t channelNum))ROM_DMATABLE[5])
#define ROM_DMA_isChannelEnabled                                              \
        ((bool (*)(uint32_t channelNum))ROM_DMATABLE[6])
#define ROM_DMA_setControlBase                                                \
        ((void (*)(void *controlTable))ROM_DMATABLE[7])
#define ROM_DMA_getControlBase                                                \
        ((void* (*)(void))ROM_DMATABLE[8])
#define ROM_DMA_getControlAlternateBase                                       \
        ((void* (*)(void))ROM_DMATABLE[9])
#define ROM_DMA_requestChannel                                                \
        ((void (*)(uint32_t channelNum))ROM_DMATABLE[10])
#define ROM_DMA_enableChannelAttribute                                        \
        ((void (*)(uint32_t channelNum,                                       \
                   uint32_t attr))ROM_DMATABLE[11])
#define ROM_DMA_disableChannelAttribute                                       \
        ((void (*)(uint32_t channelNum,                                       \
                   uint32_t attr))ROM_DMATABLE[12])
#define ROM_DMA_getChannelAttribute                                           \
        ((uint32_t (*)(uint32_t channelNum))ROM_DMATABLE[13])
#define ROM_DMA_setChannelControl                                             \
        ((void (*)(uint32_t channelStructIndex,                               \
                   uint32_t control))ROM_DMATABLE[14])
#define ROM_DMA_setChannelTransfer                                            \
        ((void (*)(uint32_t channelStructIndex,                               \
                   uint32_t mode,                                             \
                   void *srcAddr,                                             \
                   void *dstAddr,                                             \
                   uint32_t transferSize))ROM_DMATABLE[15])
#define ROM_DMA_setChannelScatterGather                                       \
        ((void (*)(uint32_t channelNum,                                       \
                   uint32_t taskCount,                                        \
                   void *taskList,                                            \
                   uint32_t isPeriphSG))ROM_DMATABLE[16])
#define ROM_DMA_getChannelSize                                                \
        ((uint32_t (*)(uint32_t channelStructIndex))ROM_DMATABLE[17])
#define ROM_DMA_getChannelMode                                                \
        ((uint32_t (*)(uint32_t channelStructIndex))ROM_DMATABLE[18])
#define ROM_DMA_assignChannel                                                 \
        ((void (*)(uint32_t mapping))ROM_DMATABLE[19])
#define ROM_DMA_requestSoftwareTransfer                                       \
        ((void (*)(uint32_t channel))ROM_DMATABLE[20])
#define ROM_DMA_assignInterrupt                                               \
        ((void (*)(uint32_t interruptNumber,                                  \
                   uint32_t channel))ROM_DMATABLE[21])
#define ROM_DMA_enableInterrupt                                               \
        ((void (*)(uint32_t interruptNumber))ROM_DMATABLE[22])
#define ROM_DMA_disableInterrupt                                              \
        ((void (*)(uint32_t interruptNumber))ROM_DMATABLE[23])
#define ROM_DMA_getInterruptStatus                                            \
        ((uint32_t (*)(void))ROM_DMATABLE[24])
#define ROM_DMA_clearInterruptFlag                                            \
        ((void (*)(uint32_t intChannel))ROM_DMATABLE[25])

//*****************************************************************************
//
// Macros for calling ROM functions in the Flash API.
//
//*****************************************************************************
#define ROM_FlashCtl_enableReadBuffering                                      \
        ((void (*)(uint_fast8_t memoryBank,                                   \
                   uint_fast8_t accessMethod))ROM_FLASHCTLTABLE[2])
#define ROM_FlashCtl_disableReadBuffering                                     \
        ((void (*)(uint_fast8_t memoryBank,                                   \
                   uint_fast8_t accessMethod))ROM_FLASHCTLTABLE[3])
#define ROM_FlashCtl_unprotectSector                                          \
        ((bool (*)(uint_fast8_t memorySpace,                                  \
                   uint32_t sectorMask))ROM_FLASHCTLTABLE[4])
#define ROM_FlashCtl_protectSector                                            \
        ((bool (*)(uint_fast8_t memorySpace,                                  \
                   uint32_t sectorMask))ROM_FLASHCTLTABLE[5])
#define ROM_FlashCtl_isSectorProtected                                        \
        ((bool (*)(uint_fast8_t memorySpace,                                  \
                   uint32_t sector))ROM_FLASHCTLTABLE[6])
#define ROM_FlashCtl_verifyMemory                                             \
        ((bool (*)(void* verifyAddr,                                          \
                   uint32_t length,                                           \
                   uint_fast8_t pattern))ROM_FLASHCTLTABLE[7])
#define ROM_FlashCtl_performMassErase                                         \
        ((bool (*)(void))ROM_FLASHCTLTABLE[8])
#define ROM_FlashCtl_eraseSector                                              \
        ((bool (*)(uint32_t addr))ROM_FLASHCTLTABLE[9])
#define ROM_FlashCtl_programMemory                                            \
        ((bool (*)(void* src,                                                 \
                   void* dest,                                                \
                   uint32_t length))ROM_FLASHCTLTABLE[10])
#define ROM_FlashCtl_setProgramVerification                                   \
        ((void (*)(uint32_t verificationSetting))ROM_FLASHCTLTABLE[11])
#define ROM_FlashCtl_clearProgramVerification                                 \
        ((void (*)(uint32_t verificationSetting))ROM_FLASHCTLTABLE[12])
#define ROM_FlashCtl_enableWordProgramming                                    \
        ((void (*)(uint32_t mode))ROM_FLASHCTLTABLE[13])
#define ROM_FlashCtl_disableWordProgramming                                   \
        ((void (*)(void))ROM_FLASHCTLTABLE[14])
#define ROM_FlashCtl_isWordProgrammingEnabled                                 \
        ((uint32_t (*)(void))ROM_FLASHCTLTABLE[15])
#define ROM_FlashCtl_enableInterrupt                                          \
        ((void (*)(uint32_t flags))ROM_FLASHCTLTABLE[16])
#define ROM_FlashCtl_disableInterrupt                                         \
        ((void (*)(uint32_t flags))ROM_FLASHCTLTABLE[17])
#define ROM_FlashCtl_getEnabledInterruptStatus                                \
        ((uint32_t (*)(void))ROM_FLASHCTLTABLE[18])
#define ROM_FlashCtl_getInterruptStatus                                       \
        ((uint32_t (*)(void))ROM_FLASHCTLTABLE[19])
#define ROM_FlashCtl_clearInterruptFlag                                       \
        ((void (*)(uint32_t flags))ROM_FLASHCTLTABLE[20])
#define ROM_FlashCtl_setWaitState                                             \
        ((void (*)(uint32_t bank,                                             \
                   uint32_t waitState))ROM_FLASHCTLTABLE[21])
#define ROM_FlashCtl_getWaitState                                             \
        ((uint32_t (*)(uint32_t bank))ROM_FLASHCTLTABLE[22])
#define ROM_FlashCtl_setReadMode                                              \
        ((bool (*)(uint32_t flashBank,                                        \
                   uint32_t readMode))ROM_FLASHCTLTABLE[23])
#define ROM_FlashCtl_getReadMode                                              \
        ((uint32_t (*)(uint32_t flashBank))ROM_FLASHCTLTABLE[24])
#define ROM___FlashCtl_remaskData8Post                                        \
        ((uint8_t (*)(uint8_t data,                                           \
                      uint32_t addr))ROM_FLASHCTLTABLE[27])
#define ROM___FlashCtl_remaskData8Pre                                         \
        ((uint8_t (*)(uint8_t data,                                           \
                      uint32_t addr))ROM_FLASHCTLTABLE[28])
#define ROM___FlashCtl_remaskData32Pre                                        \
        ((uint32_t (*)(uint32_t data,                                         \
                       uint32_t addr))ROM_FLASHCTLTABLE[29])
#define ROM___FlashCtl_remaskData32Post                                       \
        ((uint32_t (*)(uint32_t data,                                         \
                       uint32_t addr))ROM_FLASHCTLTABLE[30])
#define ROM___FlashCtl_remaskBurstDataPre                                     \
        ((void (*)(uint32_t addr,                                             \
                   uint32_t size))ROM_FLASHCTLTABLE[31])
#define ROM___FlashCtl_remaskBurstDataPost                                    \
        ((void (*)(uint32_t addr,                                             \
                   uint32_t size))ROM_FLASHCTLTABLE[32])
#define ROM_FlashCtl_initiateSectorErase                                      \
        ((void (*)(uint32_t addr))ROM_FLASHCTLTABLE[33])
#define ROM_FlashCtl_initiateMassErase                                        \
        ((void (*)(void))ROM_FLASHCTLTABLE[34])

//*****************************************************************************
//
// Macros for calling ROM functions in the FPU API.
//
//*****************************************************************************
#define ROM_FPU_enableModule                                                  \
        ((void (*)(void))ROM_FPUTABLE[0])
#define ROM_FPU_disableModule                                                 \
        ((void (*)(void))ROM_FPUTABLE[1])
#define ROM_FPU_enableStacking                                                \
        ((void (*)(void))ROM_FPUTABLE[2])
#define ROM_FPU_enableLazyStacking                                            \
        ((void (*)(void))ROM_FPUTABLE[3])
#define ROM_FPU_disableStacking                                               \
        ((void (*)(void))ROM_FPUTABLE[4])
#define ROM_FPU_setHalfPrecisionMode                                          \
        ((void (*)(uint32_t mode))ROM_FPUTABLE[5])
#define ROM_FPU_setNaNMode                                                    \
        ((void (*)(uint32_t mode))ROM_FPUTABLE[6])
#define ROM_FPU_setFlushToZeroMode                                            \
        ((void (*)(uint32_t mode))ROM_FPUTABLE[7])
#define ROM_FPU_setRoundingMode                                               \
        ((void (*)(uint32_t mode))ROM_FPUTABLE[8])

//*****************************************************************************
//
// Macros for calling ROM functions in the GPIO API.
//
//*****************************************************************************
#define ROM_GPIO_setAsOutputPin                                               \
        ((void (*)(uint_fast8_t selectedPort,                                 \
                   uint_fast16_t selectedPins))ROM_GPIOTABLE[0])
#define ROM_GPIO_setOutputHighOnPin                                           \
        ((void (*)(uint_fast8_t selectedPort,                                 \
                   uint_fast16_t selectedPins))ROM_GPIOTABLE[1])
#define ROM_GPIO_setOutputLowOnPin                                            \
        ((void (*)(uint_fast8_t selectedPort,                                 \
                   uint_fast16_t selectedPins))ROM_GPIOTABLE[2])
#define ROM_GPIO_toggleOutputOnPin                                            \
        ((void (*)(uint_fast8_t selectedPort,                                 \
                   uint_fast16_t selectedPins))ROM_GPIOTABLE[3])
#define ROM_GPIO_setAsInputPinWithPullDownResistor                            \
        ((void (*)(uint_fast8_t selectedPort,                                 \
                   uint_fast16_t selectedPins))ROM_GPIOTABLE[4])
#define ROM_GPIO_setAsInputPinWithPullUpResistor                              \
        ((void (*)(uint_fast8_t selectedPort,                                 \
                   uint_fast16_t selectedPins))ROM_GPIOTABLE[5])
#define ROM_GPIO_setAsPeripheralModuleFunctionOutputPin                       \
        ((void (*)( uint_fast8_t selectedPort,                                \
                   uint_fast16_t selectedPins,                                \
                   uint_fast8_t mode))ROM_GPIOTABLE[6])
#define ROM_GPIO_setAsPeripheralModuleFunctionInputPin                        \
        ((void (*)( uint_fast8_t selectedPort,                                \
                   uint_fast16_t selectedPins,                                \
                   uint_fast8_t mode))ROM_GPIOTABLE[7])
#define ROM_GPIO_getInputPinValue                                             \
        ((uint8_t (*)(uint_fast8_t selectedPort,                              \
                      uint_fast16_t selectedPins))ROM_GPIOTABLE[8])
#define ROM_GPIO_interruptEdgeSelect                                          \
        ((void (*)(uint_fast8_t selectedPort,                                 \
                   uint_fast16_t selectedPins,                                \
                   uint_fast8_t edgeSelect))ROM_GPIOTABLE[9])
#define ROM_GPIO_enableInterrupt                                              \
        ((void (*)(uint_fast8_t selectedPort,                                 \
                   uint_fast16_t selectedPins))ROM_GPIOTABLE[10])
#define ROM_GPIO_disableInterrupt                                             \
        ((void (*)(uint_fast8_t selectedPort,                                 \
                   uint_fast16_t selectedPins))ROM_GPIOTABLE[11])
#define ROM_GPIO_getInterruptStatus                                           \
        ((uint_fast16_t (*)(uint_fast8_t selectedPort,                        \
                            uint_fast16_t selectedPins))ROM_GPIOTABLE[12])
#define ROM_GPIO_clearInterruptFlag                                           \
        ((void (*)(uint_fast8_t selectedPort,                                 \
                   uint_fast16_t selectedPins))ROM_GPIOTABLE[13])
#define ROM_GPIO_setAsInputPin                                                \
        ((void (*)(uint_fast8_t selectedPort,                                 \
                   uint_fast16_t selectedPins))ROM_GPIOTABLE[14])
#define ROM_GPIO_getEnabledInterruptStatus                                    \
        ((uint_fast16_t (*)(uint_fast8_t selectedPort))ROM_GPIOTABLE[15])
#define ROM_GPIO_setDriveStrengthHigh                                         \
        ((void (*)(uint_fast8_t selectedPort,                                 \
                   uint_fast8_t selectedPins))ROM_GPIOTABLE[16])
#define ROM_GPIO_setDriveStrengthLow                                          \
        ((void (*)(uint_fast8_t selectedPort,                                 \
                   uint_fast8_t selectedPins))ROM_GPIOTABLE[17])

//*****************************************************************************
//
// Macros for calling ROM functions in the I2C API.
//
//*****************************************************************************
#define ROM_I2C_initMaster                                                    \
        ((void (*)(uint32_t moduleInstance,                                   \
                   const eUSCI_I2C_MasterConfig *config))ROM_I2CTABLE[0])
#define ROM_I2C_initSlave                                                     \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast16_t slaveAddress,                                \
                   uint_fast8_t slaveAddressOffset,                           \
                   uint32_t slaveOwnAddressEnable))ROM_I2CTABLE[1])
#define ROM_I2C_enableModule                                                  \
        ((void (*)(uint32_t moduleInstance))ROM_I2CTABLE[2])
#define ROM_I2C_disableModule                                                 \
        ((void (*)(uint32_t moduleInstance))ROM_I2CTABLE[3])
#define ROM_I2C_setSlaveAddress                                               \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast16_t slaveAddress))ROM_I2CTABLE[4])
#define ROM_I2C_setMode                                                       \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast8_t mode))ROM_I2CTABLE[5])
#define ROM_I2C_slavePutData                                                  \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint8_t transmitData))ROM_I2CTABLE[6])
#define ROM_I2C_slaveGetData                                                  \
        ((uint8_t (*)(uint32_t moduleInstance))ROM_I2CTABLE[7])
#define ROM_I2C_isBusBusy                                                     \
        ((uint8_t (*)(uint32_t moduleInstance))ROM_I2CTABLE[8])
#define ROM_I2C_masterSendSingleByte                                          \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint8_t txData))ROM_I2CTABLE[9])
#define ROM_I2C_masterSendSingleByteWithTimeout                               \
        ((bool (*)(uint32_t moduleInstance,                                   \
                   uint8_t txData,                                            \
                   uint32_t timeout))ROM_I2CTABLE[10])
#define ROM_I2C_masterSendMultiByteStart                                      \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint8_t txData))ROM_I2CTABLE[11])
#define ROM_I2C_masterSendMultiByteStartWithTimeout                           \
        ((bool (*)(uint32_t moduleInstance,                                   \
                   uint8_t txData,                                            \
                   uint32_t timeout))ROM_I2CTABLE[12])
#define ROM_I2C_masterSendMultiByteNext                                       \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint8_t txData))ROM_I2CTABLE[13])
#define ROM_I2C_masterSendMultiByteNextWithTimeout                            \
        ((bool (*)(uint32_t moduleInstance,                                   \
                   uint8_t txData,                                            \
                   uint32_t timeout))ROM_I2CTABLE[14])
#define ROM_I2C_masterSendMultiByteFinish                                     \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint8_t txData))ROM_I2CTABLE[15])
#define ROM_I2C_masterSendMultiByteFinishWithTimeout                          \
        ((bool (*)(uint32_t moduleInstance,                                   \
                   uint8_t txData,                                            \
                   uint32_t timeout))ROM_I2CTABLE[16])
#define ROM_I2C_masterSendMultiByteStop                                       \
        ((void (*)(uint32_t moduleInstance))ROM_I2CTABLE[17])
#define ROM_I2C_masterSendMultiByteStopWithTimeout                            \
        ((bool (*)(uint32_t moduleInstance,                                   \
                   uint32_t timeout))ROM_I2CTABLE[18])
#define ROM_I2C_masterReceiveStart                                            \
        ((void (*)(uint32_t moduleInstance))ROM_I2CTABLE[19])
#define ROM_I2C_masterReceiveMultiByteNext                                    \
        ((uint8_t (*)(uint32_t moduleInstance))ROM_I2CTABLE[20])
#define ROM_I2C_masterReceiveMultiByteFinish                                  \
        ((uint8_t (*)(uint32_t moduleInstance))ROM_I2CTABLE[21])
#define ROM_I2C_masterReceiveMultiByteFinishWithTimeout                       \
        ((bool (*)(uint32_t moduleInstance,                                   \
                   uint8_t *txData,                                           \
                   uint32_t timeout))ROM_I2CTABLE[22])
#define ROM_I2C_masterReceiveMultiByteStop                                    \
        ((void (*)(uint32_t moduleInstance))ROM_I2CTABLE[23])
#define ROM_I2C_masterReceiveSingleByte                                       \
        ((uint8_t (*)(uint32_t moduleInstance))ROM_I2CTABLE[24])
#define ROM_I2C_masterReceiveSingle                                           \
        ((uint8_t (*)(uint32_t moduleInstance))ROM_I2CTABLE[25])
#define ROM_I2C_getReceiveBufferAddressForDMA                                 \
        ((uint32_t (*)(uint32_t moduleInstance))ROM_I2CTABLE[26])
#define ROM_I2C_getTransmitBufferAddressForDMA                                \
        ((uint32_t (*)(uint32_t moduleInstance))ROM_I2CTABLE[27])
#define ROM_I2C_masterIsStopSent                                              \
        ((uint8_t (*)(uint32_t moduleInstance))ROM_I2CTABLE[28])
#define ROM_I2C_masterIsStartSent                                             \
        ((bool (*)(uint32_t moduleInstance))ROM_I2CTABLE[29])
#define ROM_I2C_masterSendStart                                               \
        ((void (*)(uint32_t moduleInstance))ROM_I2CTABLE[30])
#define ROM_I2C_enableMultiMasterMode                                         \
        ((void (*)(uint32_t moduleInstance))ROM_I2CTABLE[31])
#define ROM_I2C_disableMultiMasterMode                                        \
        ((void (*)(uint32_t moduleInstance))ROM_I2CTABLE[32])
#define ROM_I2C_enableInterrupt                                               \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast16_t mask))ROM_I2CTABLE[33])
#define ROM_I2C_disableInterrupt                                              \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast16_t mask))ROM_I2CTABLE[34])
#define ROM_I2C_clearInterruptFlag                                            \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast16_t mask))ROM_I2CTABLE[35])
#define ROM_I2C_getInterruptStatus                                            \
        ((uint_fast16_t (*)(uint32_t moduleInstance,                          \
                            uint16_t mask))ROM_I2CTABLE[36])
#define ROM_I2C_getEnabledInterruptStatus                                     \
        ((uint_fast16_t (*)(uint32_t moduleInstance))ROM_I2CTABLE[37])
#define ROM_I2C_getMode                                                       \
        ((uint_fast8_t (*)(uint32_t moduleInstance))ROM_I2CTABLE[38])

//*****************************************************************************
//
// Macros for calling ROM functions in the Interrupt API.
//
//*****************************************************************************
#define ROM_Interrupt_enableMaster                                            \
        ((bool (*)(void))ROM_INTTABLE[0])
#define ROM_Interrupt_disableMaster                                           \
        ((bool (*)(void))ROM_INTTABLE[1])
#define ROM_Interrupt_setPriorityGrouping                                     \
        ((void (*)(uint32_t bits))ROM_INTTABLE[2])
#define ROM_Interrupt_getPriorityGrouping                                     \
        ((uint32_t (*)(void))ROM_INTTABLE[3])
#define ROM_Interrupt_setPriority                                             \
        ((void (*)(uint32_t interruptNumber,                                  \
                   uint8_t priority))ROM_INTTABLE[4])
#define ROM_Interrupt_getPriority                                             \
        ((uint8_t (*)(uint32_t interruptNumber))ROM_INTTABLE[5])
#define ROM_Interrupt_enableInterrupt                                         \
        ((void (*)(uint32_t interruptNumber))ROM_INTTABLE[6])
#define ROM_Interrupt_disableInterrupt                                        \
        ((void (*)(uint32_t interruptNumber))ROM_INTTABLE[7])
#define ROM_Interrupt_isEnabled                                               \
        ((bool (*)(uint32_t interruptNumber))ROM_INTTABLE[8])
#define ROM_Interrupt_pendInterrupt                                           \
        ((void (*)(uint32_t interruptNumber))ROM_INTTABLE[9])
#define ROM_Interrupt_setPriorityMask                                         \
        ((void (*)(uint8_t priorityMask))ROM_INTTABLE[10])
#define ROM_Interrupt_getPriorityMask                                         \
        ((uint8_t (*)(void))ROM_INTTABLE[11])
#define ROM_Interrupt_setVectorTableAddress                                   \
        ((void (*)(uint32_t addr))ROM_INTTABLE[12])
#define ROM_Interrupt_getVectorTableAddress                                   \
        ((uint32_t (*)(void))ROM_INTTABLE[13])
#define ROM_Interrupt_enableSleepOnIsrExit                                    \
        ((void (*)(void))ROM_INTTABLE[14])
#define ROM_Interrupt_disableSleepOnIsrExit                                   \
        ((void (*)(void))ROM_INTTABLE[15])
#define ROM_Interrupt_unpendInterrupt                                         \
        ((void (*)(uint32_t interruptNumber))ROM_INTTABLE[18])

//*****************************************************************************
//
// Macros for calling ROM functions in the MPU API.
//
//*****************************************************************************
#define ROM_MPU_enableModule                                                  \
        ((void (*)(uint32_t mpuConfig))ROM_MPUTABLE[0])
#define ROM_MPU_disableModule                                                 \
        ((void (*)(void))ROM_MPUTABLE[1])
#define ROM_MPU_getRegionCount                                                \
        ((uint32_t (*)(void))ROM_MPUTABLE[2])
#define ROM_MPU_enableRegion                                                  \
        ((void (*)(uint32_t region))ROM_MPUTABLE[3])
#define ROM_MPU_disableRegion                                                 \
        ((void (*)(uint32_t region))ROM_MPUTABLE[4])
#define ROM_MPU_setRegion                                                     \
        ((void (*)(uint32_t region,                                           \
                   uint32_t addr,                                             \
                   uint32_t flags))ROM_MPUTABLE[5])
#define ROM_MPU_getRegion                                                     \
        ((void (*)(uint32_t region,                                           \
                   uint32_t *addr,                                            \
                   uint32_t *pflags))ROM_MPUTABLE[6])
#define ROM_MPU_enableInterrupt                                               \
        ((void (*)(void))ROM_MPUTABLE[7])
#define ROM_MPU_disableInterrupt                                              \
        ((void (*)(void))ROM_MPUTABLE[8])

//*****************************************************************************
//
// Macros for calling ROM functions in the PCM API.
//
//*****************************************************************************
#define ROM_PCM_setCoreVoltageLevel                                           \
        ((bool (*)(uint_fast8_t voltageLevel))ROM_PCMTABLE[0])
#define ROM_PCM_getCoreVoltageLevel                                           \
        ((uint8_t (*)(void))ROM_PCMTABLE[1])
#define ROM_PCM_setCoreVoltageLevelWithTimeout                                \
        ((bool (*)(uint_fast8_t voltageLevel,                                 \
                   uint32_t timeOut))ROM_PCMTABLE[2])
#define ROM_PCM_setPowerMode                                                  \
        ((bool (*)(uint_fast8_t powerMode))ROM_PCMTABLE[3])
#define ROM_PCM_setPowerModeWithTimeout                                       \
        ((bool (*)(uint_fast8_t powerMode,                                    \
                   uint32_t timeOut))ROM_PCMTABLE[4])
#define ROM_PCM_getPowerMode                                                  \
        ((uint8_t (*)(void))ROM_PCMTABLE[5])
#define ROM_PCM_setPowerState                                                 \
        ((bool (*)(uint_fast8_t powerState))ROM_PCMTABLE[6])
#define ROM_PCM_setPowerStateWithTimeout                                      \
        ((bool (*)(uint_fast8_t powerState,                                   \
                   uint32_t timeout))ROM_PCMTABLE[7])
#define ROM_PCM_getPowerState                                                 \
        ((uint8_t (*)(void))ROM_PCMTABLE[8])
#define ROM_PCM_shutdownDevice                                                \
        ((bool (*)(uint32_t shutdownMode))ROM_PCMTABLE[9])
#define ROM_PCM_gotoLPM0                                                      \
        ((bool (*)(void))ROM_PCMTABLE[10])
#define ROM_PCM_gotoLPM3                                                      \
        ((bool (*)(void))ROM_PCMTABLE[11])
#define ROM_PCM_enableInterrupt                                               \
        ((void (*)(uint32_t flags))ROM_PCMTABLE[12])
#define ROM_PCM_disableInterrupt                                              \
        ((void (*)(uint32_t flags))ROM_PCMTABLE[13])
#define ROM_PCM_getInterruptStatus                                            \
        ((uint32_t (*)(void))ROM_PCMTABLE[14])
#define ROM_PCM_getEnabledInterruptStatus                                     \
        ((uint32_t (*)(void))ROM_PCMTABLE[15])
#define ROM_PCM_clearInterruptFlag                                            \
        ((void (*)(uint32_t flags))ROM_PCMTABLE[16])
#define ROM_PCM_enableRudeMode                                                \
        ((void (*)(void))ROM_PCMTABLE[17])
#define ROM_PCM_disableRudeMode                                               \
        ((void (*)(void))ROM_PCMTABLE[18])
#define ROM_PCM_gotoLPM0InterruptSafe                                         \
        ((bool (*)(void))ROM_PCMTABLE[19])
#define ROM_PCM_gotoLPM3InterruptSafe                                         \
        ((bool (*)(void))ROM_PCMTABLE[20])
#define ROM_PCM_setCoreVoltageLevelNonBlocking                                \
        ((bool (*)(uint_fast8_t voltageLevel))ROM_PCMTABLE[23])
#define ROM_PCM_setPowerModeNonBlocking                                       \
        ((bool (*)(uint_fast8_t powerMode))ROM_PCMTABLE[24])
#define ROM_PCM_setPowerStateNonBlocking                                      \
        ((bool (*)(uint_fast8_t powerState))ROM_PCMTABLE[25])
#define ROM_PCM_gotoLPM4                                                      \
        ((bool (*)(void))ROM_PCMTABLE[26])
#define ROM_PCM_gotoLPM4InterruptSafe                                         \
        ((bool (*)(void))ROM_PCMTABLE[27])

//*****************************************************************************
//
// Macros for calling ROM functions in the PMAP API.
//
//*****************************************************************************
#define ROM_PMAP_configurePorts                                               \
        ((void (*)(const uint8_t *portMapping,                                \
                   uint8_t pxMAPy,                                            \
                   uint8_t numberOfPorts,                                     \
                   uint8_t portMapReconfigure))ROM_PMAPTABLE[0])

//*****************************************************************************
//
// Macros for calling ROM functions in the PSS API.
//
//*****************************************************************************
#define ROM_PSS_enableHighSidePinToggle                                       \
        ((void (*)(bool activeLow))ROM_PSSTABLE[0])
#define ROM_PSS_disableHighSidePinToggle                                      \
        ((void (*)(void))ROM_PSSTABLE[1])
#define ROM_PSS_enableHighSide                                                \
        ((void (*)(void))ROM_PSSTABLE[2])
#define ROM_PSS_disableHighSide                                               \
        ((void (*)(void))ROM_PSSTABLE[3])
#define ROM_PSS_setHighSidePerformanceMode                                    \
        ((void (*)(uint_fast8_t powerMode))ROM_PSSTABLE[6])
#define ROM_PSS_getHighSidePerformanceMode                                    \
        ((uint_fast8_t (*)(void))ROM_PSSTABLE[7])
#define ROM_PSS_enableHighSideMonitor                                         \
        ((void (*)(void))ROM_PSSTABLE[10])
#define ROM_PSS_disableHighSideMonitor                                        \
        ((void (*)(void))ROM_PSSTABLE[11])
#define ROM_PSS_setHighSideVoltageTrigger                                     \
        ((void (*)(uint_fast8_t triggerVoltage))ROM_PSSTABLE[12])
#define ROM_PSS_getHighSideVoltageTrigger                                     \
        ((uint_fast8_t (*)(void))ROM_PSSTABLE[13])
#define ROM_PSS_enableInterrupt                                               \
        ((void (*)(void))ROM_PSSTABLE[14])
#define ROM_PSS_disableInterrupt                                              \
        ((void (*)(void))ROM_PSSTABLE[15])
#define ROM_PSS_getInterruptStatus                                            \
        ((uint32_t (*)(void))ROM_PSSTABLE[16])
#define ROM_PSS_clearInterruptFlag                                            \
        ((void (*)(void))ROM_PSSTABLE[17])
#define ROM_PSS_enableForcedDCDCOperation                                     \
        ((void (*)(void))ROM_PSSTABLE[20])
#define ROM_PSS_disableForcedDCDCOperation                                    \
        ((void (*)(void))ROM_PSSTABLE[21])

//*****************************************************************************
//
// Macros for calling ROM functions in the Ref API.
//
//*****************************************************************************
#define ROM_REF_A_setReferenceVoltage                                         \
        ((void (*)(uint_fast8_t referenceVoltageSelect))ROM_REFTABLE[0])
#define ROM_REF_A_disableTempSensor                                           \
        ((void (*)(void))ROM_REFTABLE[1])
#define ROM_REF_A_enableTempSensor                                            \
        ((void (*)(void))ROM_REFTABLE[2])
#define ROM_REF_A_enableReferenceVoltageOutput                                \
        ((void (*)(void))ROM_REFTABLE[3])
#define ROM_REF_A_disableReferenceVoltageOutput                               \
        ((void (*)(void))ROM_REFTABLE[4])
#define ROM_REF_A_enableReferenceVoltage                                      \
        ((void (*)(void))ROM_REFTABLE[5])
#define ROM_REF_A_disableReferenceVoltage                                     \
        ((void (*)(void))ROM_REFTABLE[6])
#define ROM_REF_A_getBandgapMode                                              \
        ((uint_fast8_t (*)(void))ROM_REFTABLE[7])
#define ROM_REF_A_isBandgapActive                                             \
        ((bool (*)(void))ROM_REFTABLE[8])
#define ROM_REF_A_isRefGenBusy                                                \
        ((bool (*)(void))ROM_REFTABLE[9])
#define ROM_REF_A_isRefGenActive                                              \
        ((bool (*)(void))ROM_REFTABLE[10])
#define ROM_REF_A_getBufferedBandgapVoltageStatus                             \
        ((bool (*)(void))ROM_REFTABLE[11])
#define ROM_REF_A_getVariableReferenceVoltageStatus                           \
        ((bool (*)(void))ROM_REFTABLE[12])
#define ROM_REF_A_setReferenceVoltageOneTimeTrigger                           \
        ((void (*)(void))ROM_REFTABLE[13])
#define ROM_REF_A_setBufferedBandgapVoltageOneTimeTrigger                     \
        ((void (*)(void))ROM_REFTABLE[14])

//*****************************************************************************
//
// Macros for calling ROM functions in the ResetCtl API.
//
//*****************************************************************************
#define ROM_ResetCtl_initiateSoftReset                                        \
        ((void (*)(void))ROM_RESETCTLTABLE[0])
#define ROM_ResetCtl_initiateSoftResetWithSource                              \
        ((void (*)(uint32_t source))ROM_RESETCTLTABLE[1])
#define ROM_ResetCtl_getSoftResetSource                                       \
        ((uint32_t (*)(void))ROM_RESETCTLTABLE[2])
#define ROM_ResetCtl_clearSoftResetSource                                     \
        ((void (*)(uint32_t mask))ROM_RESETCTLTABLE[3])
#define ROM_ResetCtl_initiateHardReset                                        \
        ((void (*)(void))ROM_RESETCTLTABLE[4])
#define ROM_ResetCtl_initiateHardResetWithSource                              \
        ((void (*)(uint32_t source))ROM_RESETCTLTABLE[5])
#define ROM_ResetCtl_getHardResetSource                                       \
        ((uint32_t (*)(void))ROM_RESETCTLTABLE[6])
#define ROM_ResetCtl_clearHardResetSource                                     \
        ((void (*)(uint32_t mask))ROM_RESETCTLTABLE[7])
#define ROM_ResetCtl_getPSSSource                                             \
        ((uint32_t (*)(void))ROM_RESETCTLTABLE[8])
#define ROM_ResetCtl_clearPSSFlags                                            \
        ((void (*)(void))ROM_RESETCTLTABLE[9])
#define ROM_ResetCtl_getPCMSource                                             \
        ((uint32_t (*)(void))ROM_RESETCTLTABLE[10])
#define ROM_ResetCtl_clearPCMFlags                                            \
        ((void (*)(void))ROM_RESETCTLTABLE[11])

//*****************************************************************************
//
// Macros for calling ROM functions in the RTC API.
//
//*****************************************************************************
#define ROM_RTC_C_startClock                                                  \
        ((void (*)(void))ROM_RTCTABLE[0])
#define ROM_RTC_C_holdClock                                                   \
        ((void (*)(void))ROM_RTCTABLE[1])
#define ROM_RTC_C_setCalibrationFrequency                                     \
        ((void (*)(uint_fast16_t frequencySelect))ROM_RTCTABLE[2])
#define ROM_RTC_C_setCalibrationData                                          \
        ((void (*)(uint_fast8_t offsetDirection,                              \
                   uint_fast8_t offsetValue))ROM_RTCTABLE[3])
#define ROM_RTC_C_setTemperatureCompensation                                  \
        ((bool (*)(uint_fast16_t offsetDirection,                             \
                   uint_fast8_t offsetValue))ROM_RTCTABLE[4])
#define ROM_RTC_C_initCalendar                                                \
        ((void (*)(const RTC_C_Calendar *calendarTime,                        \
                   uint_fast16_t formatSelect))ROM_RTCTABLE[5])
#define ROM_RTC_C_getCalendarTime                                             \
        ((RTC_C_Calendar (*)(void))ROM_RTCTABLE[6])
#define ROM_RTC_C_setCalendarAlarm                                            \
        ((void (*)(uint_fast8_t minutesAlarm,                                 \
                   uint_fast8_t hoursAlarm,                                   \
                   uint_fast8_t dayOfWeekAlarm,                               \
                   uint_fast8_t dayOfmonthAlarm))ROM_RTCTABLE[7])
#define ROM_RTC_C_setCalendarEvent                                            \
        ((void (*)(uint_fast16_t eventSelect))ROM_RTCTABLE[8])
#define ROM_RTC_C_definePrescaleEvent                                         \
        ((void (*)(uint_fast8_t prescaleSelect,                               \
                   uint_fast8_t prescaleEventDivider))ROM_RTCTABLE[9])
#define ROM_RTC_C_getPrescaleValue                                            \
        ((uint_fast8_t (*)(uint_fast8_t prescaleSelect))ROM_RTCTABLE[10])
#define ROM_RTC_C_setPrescaleValue                                            \
        ((void (*)(uint_fast8_t prescaleSelect,                               \
                   uint_fast8_t prescaleCounterValue))ROM_RTCTABLE[11])
#define ROM_RTC_C_convertBCDToBinary                                          \
        ((uint16_t (*)(uint16_t valueToConvert))ROM_RTCTABLE[12])
#define ROM_RTC_C_convertBinaryToBCD                                          \
        ((uint16_t (*)(uint16_t valueToConvert))ROM_RTCTABLE[13])
#define ROM_RTC_C_enableInterrupt                                             \
        ((void (*)(uint8_t interruptMask))ROM_RTCTABLE[14])
#define ROM_RTC_C_disableInterrupt                                            \
        ((void (*)(uint8_t interruptMask))ROM_RTCTABLE[15])
#define ROM_RTC_C_getInterruptStatus                                          \
        ((uint_fast8_t (*)(void))ROM_RTCTABLE[16])
#define ROM_RTC_C_getEnabledInterruptStatus                                   \
        ((uint_fast8_t (*)(void))ROM_RTCTABLE[17])
#define ROM_RTC_C_clearInterruptFlag                                          \
        ((void (*)(uint_fast8_t interruptFlagMask))ROM_RTCTABLE[18])

//*****************************************************************************
//
// Macros for calling ROM functions in the SPI API.
//
//*****************************************************************************
#define ROM_SPI_initMaster                                                    \
        ((bool (*)(uint32_t moduleInstance,                                   \
                   const eUSCI_SPI_MasterConfig *config))ROM_SPITABLE[0])
#define ROM_SPI_selectFourPinFunctionality                                    \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast8_t select4PinFunctionality))ROM_SPITABLE[1])
#define ROM_SPI_changeMasterClock                                             \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint32_t clockSourceFrequency,                             \
                   uint32_t desiredSpiClock))ROM_SPITABLE[2])
#define ROM_SPI_initSlave                                                     \
        ((bool (*)(uint32_t moduleInstance,                                   \
                   const eUSCI_SPI_SlaveConfig *config))ROM_SPITABLE[3])
#define ROM_SPI_changeClockPhasePolarity                                      \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast16_t clockPhase,                                  \
                   uint_fast16_t clockPolarity))ROM_SPITABLE[4])
#define ROM_SPI_transmitData                                                  \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast8_t transmitData))ROM_SPITABLE[5])
#define ROM_SPI_receiveData                                                   \
        ((uint8_t (*)(uint32_t moduleInstance))ROM_SPITABLE[6])
#define ROM_SPI_enableModule                                                  \
        ((void (*)(uint32_t moduleInstance))ROM_SPITABLE[7])
#define ROM_SPI_disableModule                                                 \
        ((void (*)(uint32_t moduleInstance))ROM_SPITABLE[8])
#define ROM_SPI_getReceiveBufferAddressForDMA                                 \
        ((uint32_t (*)(uint32_t moduleInstance))ROM_SPITABLE[9])
#define ROM_SPI_getTransmitBufferAddressForDMA                                \
        ((uint32_t (*)(uint32_t moduleInstance))ROM_SPITABLE[10])
#define ROM_SPI_isBusy                                                        \
        ((uint_fast8_t (*)(uint32_t moduleInstance))ROM_SPITABLE[11])
#define ROM_SPI_enableInterrupt                                               \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast8_t mask))ROM_SPITABLE[12])
#define ROM_SPI_disableInterrupt                                              \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast8_t mask))ROM_SPITABLE[13])
#define ROM_SPI_getInterruptStatus                                            \
        ((uint_fast8_t (*)(uint32_t moduleInstance,                           \
                           uint16_t mask))ROM_SPITABLE[14])
#define ROM_SPI_getEnabledInterruptStatus                                     \
        ((uint_fast8_t (*)(uint32_t moduleInstance))ROM_SPITABLE[15])
#define ROM_SPI_clearInterruptFlag                                            \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast8_t mask))ROM_SPITABLE[16])

//*****************************************************************************
//
// Macros for calling ROM functions in the SysCtl API.
//
//*****************************************************************************
#define ROM_SysCtl_getSRAMSize                                                \
        ((uint_least32_t (*)(void))ROM_SYSCTLTABLE[0])
#define ROM_SysCtl_getFlashSize                                               \
        ((uint_least32_t (*)(void))ROM_SYSCTLTABLE[1])
#define ROM_SysCtl_rebootDevice                                               \
        ((void (*)(void))ROM_SYSCTLTABLE[2])
#define ROM_SysCtl_enableSRAMBank                                             \
        ((void (*)(uint_fast8_t sramBank))ROM_SYSCTLTABLE[3])
#define ROM_SysCtl_disableSRAMBank                                            \
        ((void (*)(uint_fast8_t sramBank))ROM_SYSCTLTABLE[4])
#define ROM_SysCtl_enableSRAMBankRetention                                    \
        ((void (*)(uint_fast8_t sramBank))ROM_SYSCTLTABLE[5])
#define ROM_SysCtl_disableSRAMBankRetention                                   \
        ((void (*)(uint_fast8_t sramBank))ROM_SYSCTLTABLE[6])
#define ROM_SysCtl_enablePeripheralAtCPUHalt                                  \
        ((void (*)(uint_fast16_t devices))ROM_SYSCTLTABLE[7])
#define ROM_SysCtl_disablePeripheralAtCPUHalt                                 \
        ((void (*)(uint_fast16_t devices))ROM_SYSCTLTABLE[8])
#define ROM_SysCtl_setWDTTimeoutResetType                                     \
        ((void (*)(uint_fast8_t resetType))ROM_SYSCTLTABLE[9])
#define ROM_SysCtl_setWDTPasswordViolationResetType                           \
        ((void (*)(uint_fast8_t resetType))ROM_SYSCTLTABLE[10])
#define ROM_SysCtl_disableNMISource                                           \
        ((void (*)(uint_fast8_t flags))ROM_SYSCTLTABLE[11])
#define ROM_SysCtl_enableNMISource                                            \
        ((void (*)(uint_fast8_t flags))ROM_SYSCTLTABLE[12])
#define ROM_SysCtl_getNMISourceStatus                                         \
        ((uint_fast8_t (*)(void))ROM_SYSCTLTABLE[13])
#define ROM_SysCtl_getTempCalibrationConstant                                 \
        ((uint_fast16_t (*)(uint32_t refVoltage,                              \
                            uint32_t temperature))ROM_SYSCTLTABLE[14])
#define ROM_SysCtl_enableGlitchFilter                                         \
        ((void (*)(void))ROM_SYSCTLTABLE[15])
#define ROM_SysCtl_disableGlitchFilter                                        \
        ((void (*)(void))ROM_SYSCTLTABLE[16])
#define ROM_SysCtl_getTLVInfo                                                 \
        ((void (*)(uint_fast8_t tag,                                          \
                   uint_fast8_t instance,                                     \
                   uint_fast8_t *length,                                      \
                   uint32_t **data_address))ROM_SYSCTLTABLE[17])

//*****************************************************************************
//
// Macros for calling ROM functions in the SysTick API.
//
//*****************************************************************************
#define ROM_SysTick_enableModule                                              \
        ((void (*)(void))ROM_SYSTICKTABLE[0])
#define ROM_SysTick_disableModule                                             \
        ((void (*)(void))ROM_SYSTICKTABLE[1])
#define ROM_SysTick_enableInterrupt                                           \
        ((void (*)(void))ROM_SYSTICKTABLE[2])
#define ROM_SysTick_disableInterrupt                                          \
        ((void (*)(void))ROM_SYSTICKTABLE[3])
#define ROM_SysTick_setPeriod                                                 \
        ((void (*)(uint32_t period))ROM_SYSTICKTABLE[4])
#define ROM_SysTick_getPeriod                                                 \
        ((uint32_t (*)(void))ROM_SYSTICKTABLE[5])
#define ROM_SysTick_getValue                                                  \
        ((uint32_t (*)(void))ROM_SYSTICKTABLE[6])

//*****************************************************************************
//
// Macros for calling ROM functions in the Timer_A API.
//
//*****************************************************************************
#define ROM_Timer_A_startCounter                                              \
        ((void (*)(uint32_t timer,                                            \
                   uint_fast16_t timerMode))ROM_TIMER_ATABLE[0])
#define ROM_Timer_A_configureContinuousMode                                   \
        ((void (*)(uint32_t timer,                                            \
                   const Timer_A_ContinuousModeConfig *config))ROM_TIMER_ATABLE[1])
#define ROM_Timer_A_configureUpMode                                           \
        ((void (*)(uint32_t timer,                                            \
                   const Timer_A_UpModeConfig *config))ROM_TIMER_ATABLE[2])
#define ROM_Timer_A_configureUpDownMode                                       \
        ((void (*)(uint32_t timer,                                            \
                   const Timer_A_UpDownModeConfig *config))ROM_TIMER_ATABLE[3])
#define ROM_Timer_A_initCapture                                               \
        ((void (*)(uint32_t timer,                                            \
                   const Timer_A_CaptureModeConfig *config))ROM_TIMER_ATABLE[4])
#define ROM_Timer_A_initCompare                                               \
        ((void (*)(uint32_t timer,                                            \
                   const Timer_A_CompareModeConfig *config))ROM_TIMER_ATABLE[5])
#define ROM_Timer_A_clearTimer                                                \
        ((void (*)(uint32_t timer))ROM_TIMER_ATABLE[6])
#define ROM_Timer_A_getSynchronizedCaptureCompareInput                        \
        ((uint_fast8_t (*)(uint32_t timer,                                    \
                           uint_fast16_t captureCompareRegister,              \
                           uint_fast16_t synchronizedSetting))ROM_TIMER_ATABLE[7])
#define ROM_Timer_A_getOutputForOutputModeOutBitValue                         \
        ((uint_fast8_t (*)(uint32_t timer,                                    \
                           uint_fast16_t captureCompareRegister))ROM_TIMER_ATABLE[8])
#define ROM_Timer_A_getCaptureCompareCount                                    \
        ((uint_fast16_t (*)(uint32_t timer,                                   \
                            uint_fast16_t captureCompareRegister))ROM_TIMER_ATABLE[9])
#define ROM_Timer_A_generatePWM                                               \
        ((void (*)(uint32_t timer,                                            \
                   const Timer_A_PWMConfig *config))ROM_TIMER_ATABLE[11])
#define ROM_Timer_A_stopTimer                                                 \
        ((void (*)(uint32_t timer))ROM_TIMER_ATABLE[12])
#define ROM_Timer_A_setCompareValue                                           \
        ((void (*)(uint32_t timer,                                            \
                   uint_fast16_t compareRegister,                             \
                   uint_fast16_t compareValue))ROM_TIMER_ATABLE[13])
#define ROM_Timer_A_clearInterruptFlag                                        \
        ((void (*)(uint32_t timer))ROM_TIMER_ATABLE[14])
#define ROM_Timer_A_clearCaptureCompareInterrupt                              \
        ((void (*)(uint32_t timer,                                            \
                   uint_fast16_t captureCompareRegister))ROM_TIMER_ATABLE[15])
#define ROM_Timer_A_enableInterrupt                                           \
        ((void (*)(uint32_t timer))ROM_TIMER_ATABLE[16])
#define ROM_Timer_A_disableInterrupt                                          \
        ((void (*)(uint32_t timer))ROM_TIMER_ATABLE[17])
#define ROM_Timer_A_getInterruptStatus                                        \
        ((uint32_t (*)(uint32_t timer))ROM_TIMER_ATABLE[18])
#define ROM_Timer_A_getEnabledInterruptStatus                                 \
        ((uint32_t (*)(uint32_t timer))ROM_TIMER_ATABLE[19])
#define ROM_Timer_A_enableCaptureCompareInterrupt                             \
        ((void (*)(uint32_t timer,                                            \
                   uint_fast16_t captureCompareRegister))ROM_TIMER_ATABLE[20])
#define ROM_Timer_A_disableCaptureCompareInterrupt                            \
        ((void (*)(uint32_t timer,                                            \
                   uint_fast16_t captureCompareRegister))ROM_TIMER_ATABLE[21])
#define ROM_Timer_A_getCaptureCompareInterruptStatus                          \
        ((uint32_t (*)(uint32_t timer,                                        \
                       uint_fast16_t captureCompareRegister,                  \
                       uint_fast16_t mask))ROM_TIMER_ATABLE[22])
#define ROM_Timer_A_getCaptureCompareEnabledInterruptStatus                   \
        ((uint32_t (*)(uint32_t timer,                                        \
                       uint_fast16_t captureCompareRegister))ROM_TIMER_ATABLE[23])
#define ROM_Timer_A_getCounterValue                                           \
        ((uint16_t (*)(uint32_t timer))ROM_TIMER_ATABLE[26])

//*****************************************************************************
//
// Macros for calling ROM functions in the Timer32 API.
//
//*****************************************************************************
#define ROM_Timer32_initModule                                                \
        ((void (*)(uint32_t timer,                                            \
                   uint32_t preScaler,                                        \
                   uint32_t resolution,                                       \
                   uint32_t mode))ROM_TIMER32TABLE[0])
#define ROM_Timer32_setCount                                                  \
        ((void (*)(uint32_t timer,                                            \
                   uint32_t count))ROM_TIMER32TABLE[1])
#define ROM_Timer32_setCountInBackground                                      \
        ((void (*)(uint32_t timer,                                            \
                   uint32_t count))ROM_TIMER32TABLE[2])
#define ROM_Timer32_getValue                                                  \
        ((uint32_t (*)(uint32_t timer))ROM_TIMER32TABLE[3])
#define ROM_Timer32_startTimer                                                \
        ((void (*)(uint32_t timer,                                            \
                   bool oneShot))ROM_TIMER32TABLE[4])
#define ROM_Timer32_haltTimer                                                 \
        ((void (*)(uint32_t timer))ROM_TIMER32TABLE[5])
#define ROM_Timer32_enableInterrupt                                           \
        ((void (*)(uint32_t timer))ROM_TIMER32TABLE[6])
#define ROM_Timer32_disableInterrupt                                          \
        ((void (*)(uint32_t timer))ROM_TIMER32TABLE[7])
#define ROM_Timer32_clearInterruptFlag                                        \
        ((void (*)(uint32_t timer))ROM_TIMER32TABLE[8])
#define ROM_Timer32_getInterruptStatus                                        \
        ((uint32_t (*)(uint32_t timer))ROM_TIMER32TABLE[9])

//*****************************************************************************
//
// Macros for calling ROM functions in the UART API.
//
//*****************************************************************************
#define ROM_UART_initModule                                                   \
        ((bool (*)(uint32_t moduleInstance,                                   \
                   const eUSCI_UART_Config *config))ROM_UARTTABLE[0])
#define ROM_UART_transmitData                                                 \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast8_t transmitData))ROM_UARTTABLE[1])
#define ROM_UART_enableModule                                                 \
        ((void (*)(uint32_t moduleInstance))ROM_UARTTABLE[2])
#define ROM_UART_disableModule                                                \
        ((void (*)(uint32_t moduleInstance))ROM_UARTTABLE[3])
#define ROM_UART_queryStatusFlags                                             \
        ((uint_fast8_t (*)(uint32_t moduleInstance,                           \
                           uint_fast8_t mask))ROM_UARTTABLE[4])
#define ROM_UART_setDormant                                                   \
        ((void (*)(uint32_t moduleInstance))ROM_UARTTABLE[5])
#define ROM_UART_resetDormant                                                 \
        ((void (*)(uint32_t moduleInstance))ROM_UARTTABLE[6])
#define ROM_UART_transmitAddress                                              \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast8_t transmitAddress))ROM_UARTTABLE[7])
#define ROM_UART_transmitBreak                                                \
        ((void (*)(uint32_t moduleInstance))ROM_UARTTABLE[8])
#define ROM_UART_getReceiveBufferAddressForDMA                                \
        ((uint32_t (*)(uint32_t moduleInstance))ROM_UARTTABLE[9])
#define ROM_UART_getTransmitBufferAddressForDMA                               \
        ((uint32_t (*)(uint32_t moduleInstance))ROM_UARTTABLE[10])
#define ROM_UART_selectDeglitchTime                                           \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint32_t deglitchTime))ROM_UARTTABLE[11])
#define ROM_UART_enableInterrupt                                              \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast8_t mask))ROM_UARTTABLE[12])
#define ROM_UART_disableInterrupt                                             \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast8_t mask))ROM_UARTTABLE[13])
#define ROM_UART_getInterruptStatus                                           \
        ((uint_fast8_t (*)(uint32_t moduleInstance,                           \
                           uint8_t mask))ROM_UARTTABLE[14])
#define ROM_UART_clearInterruptFlag                                           \
        ((void (*)(uint32_t moduleInstance,                                   \
                   uint_fast8_t mask))ROM_UARTTABLE[15])
#define ROM_UART_receiveData                                                  \
        ((uint8_t (*)(uint32_t moduleInstance))ROM_UARTTABLE[16])
#define ROM_UART_getEnabledInterruptStatus                                    \
        ((uint_fast8_t (*)(uint32_t moduleInstance))ROM_UARTTABLE[17])

//*****************************************************************************
//
// Macros for calling ROM functions in the WDT API.
//
//*****************************************************************************
#define ROM_WDT_A_holdTimer                                                   \
        ((void (*)(void))ROM_WDTTABLE[0])
#define ROM_WDT_A_startTimer                                                  \
        ((void (*)(void))ROM_WDTTABLE[1])
#define ROM_WDT_A_clearTimer                                                  \
        ((void (*)(void))ROM_WDTTABLE[2])
#define ROM_WDT_A_initWatchdogTimer                                           \
        ((void (*)(uint_fast8_t clockSelect,                                  \
                   uint_fast8_t clockDivider))ROM_WDTTABLE[3])
#define ROM_WDT_A_initIntervalTimer                                           \
        ((void (*)(uint_fast8_t clockSelect,                                  \
                   uint_fast8_t clockDivider))ROM_WDTTABLE[4])
#define ROM_WDT_A_setPasswordViolationReset                                   \
        ((void (*)(uint_fast8_t resetType))ROM_WDTTABLE[7])
#define ROM_WDT_A_setTimeoutReset                                             \
        ((void (*)(uint_fast8_t resetType))ROM_WDTTABLE[8])

#endif  //__MSP432_DVRLIB_ROM__
#endif // __ROM_H__
