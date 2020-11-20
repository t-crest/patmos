/*
 * Copyright (c) 2007, Vanderbilt University
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
 * - Neither the name of the copyright holder nor the names of
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
 * Author: Miklos Maroti
 */

configuration RadioSnifferC
{
}

implementation
{
	#define UQ_METADATA_FLAGS	"UQ_METADATA_FLAGS"
	#define UQ_RADIO_ALARM		"UQ_RADIO_ALARM"

	components RadioSnifferP, MainC, SerialActiveMessageC, AssertC;
	
	RadioSnifferP.Boot -> MainC;
	RadioSnifferP.SplitControl -> SerialActiveMessageC;
	RadioSnifferP.RadioState -> RadioDriverLayerC;

	// just to avoid a timer compilation bug
	components new TimerMilliC();

// -------- ActiveMessage

	components new Ieee154PacketLayerC();
	Ieee154PacketLayerC.SubPacket -> TimeStampingLayerC;

// -------- TimeStamping

	components new TimeStampingLayerC();
	TimeStampingLayerC.LocalTimeRadio -> RadioDriverLayerC;
	TimeStampingLayerC.SubPacket -> MetadataFlagsLayerC;
	TimeStampingLayerC.TimeStampFlag -> MetadataFlagsLayerC.PacketFlag[unique(UQ_METADATA_FLAGS)];

// -------- MetadataFlags

	components new MetadataFlagsLayerC();
	MetadataFlagsLayerC.SubPacket -> RadioDriverLayerC;

// -------- Tasklet

	components new TaskletC();

// -------- RadioAlarm

#if !defined(PLATFORM_UCMINI) && !defined(PLATFORM_UCPROTONB) && !defined(PLATFORM_UCBASE)
	components new RadioAlarmC();
	RadioAlarmC.Alarm -> RadioDriverLayerC;
	RadioAlarmC.Tasklet -> TaskletC;
#endif

// -------- RadioDriver

#if defined(PLATFORM_IRIS) || defined(PLATFORM_MULLE) || defined(PLATFORM_MESHBEAN)
	components RF230DriverLayerC as RadioDriverLayerC;
	components RF230RadioP as RadioP;
#elif defined(PLATFORM_MESHBEAN900)
	components RF212DriverLayerC as RadioDriverLayerC;
	components RF212RadioP as RadioP;
#elif defined(PLATFORM_MICAZ) || defined(PLATFORM_TELOSA) || defined(PLATFORM_TELOSB)
	components CC2420XDriverLayerC as RadioDriverLayerC;
	components CC2420XRadioP as RadioP;
#elif defined(PLATFORM_UCMINI) || defined(PLATFORM_UCPROTONB) || defined(PLATFORM_UCBASE)
	components RFA1DriverLayerC as RadioDriverLayerC;
	components RFA1RadioP as RadioP;
#endif

	RadioDriverLayerC.PacketTimeStamp -> TimeStampingLayerC;
	RadioDriverLayerC.Config -> RadioP;
	RadioP.Ieee154PacketLayer -> Ieee154PacketLayerC;

	RadioDriverLayerC.TransmitPowerFlag -> MetadataFlagsLayerC.PacketFlag[unique(UQ_METADATA_FLAGS)];
	RadioDriverLayerC.RSSIFlag -> MetadataFlagsLayerC.PacketFlag[unique(UQ_METADATA_FLAGS)];
	RadioDriverLayerC.TimeSyncFlag -> MetadataFlagsLayerC.PacketFlag[unique(UQ_METADATA_FLAGS)];
	RadioDriverLayerC.Tasklet -> TaskletC;
#if !defined(PLATFORM_UCMINI) && !defined(PLATFORM_UCPROTONB) && !defined(PLATFORM_UCBASE)
	RadioDriverLayerC.RadioAlarm -> RadioAlarmC.RadioAlarm[unique(UQ_RADIO_ALARM)];
#endif
}
