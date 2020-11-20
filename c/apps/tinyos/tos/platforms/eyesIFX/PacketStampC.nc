/* -*- mode:c++; indent-tabs-mode:nil -*- 
 * Copyright (c) 2008, Technische Universitaet Berlin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in the 
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the Technische Universitaet Berlin nor the names 
 *   of its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

configuration PacketStampC {
    provides {
        interface TimeSyncPacket<T32khz, uint32_t> as TimeSyncPacket32khz;
        interface PacketTimeStamp<T32khz, uint32_t> as PacketTimeStamp32khz;
        
        interface TimeSyncPacket<TMilli, uint32_t> as TimeSyncPacketMilli;
        interface PacketTimeStamp<TMilli, uint32_t> as PacketTimeStampMilli;
    }
}
implementation  {
    components PacketStampP as PS;
    components LocalTimeC as LT;
    components HilTimerMilliC as HilMilli;    

    PacketTimeStamp32khz = PS;
    TimeSyncPacket32khz = PS;
    
    PacketTimeStampMilli = PS;
    TimeSyncPacketMilli = PS;
    
    PS.LocalTime32khz -> LT;
    PS.LocalTimeMilli -> HilMilli;
}

