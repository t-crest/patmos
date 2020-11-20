/** Copyright (c) 2009, University of Szeged
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* - Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* - Redistributions in binary form must reproduce the above
* copyright notice, this list of conditions and the following
* disclaimer in the documentation and/or other materials provided
* with the distribution.
* - Neither the name of University of Szeged nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Author: Zoltan Kincses
*/

#include"Accel202.h"
#include"Adg715.h"

configuration HplAccel202C {
  provides interface Resource[ uint8_t id ];
}
implementation {
	components HplAccel202P;
	components new FcfsArbiterC( UQ_ACCEL202 ) as Arbiter;
	Resource = Arbiter;
  
	components new SplitControlPowerManagerC();
	SplitControlPowerManagerC.SplitControl -> HplAccel202P;
	SplitControlPowerManagerC.ArbiterInfo -> Arbiter.ArbiterInfo;
	SplitControlPowerManagerC.ResourceDefaultOwner -> Arbiter.ResourceDefaultOwner;
	
	components Adg715C;
	HplAccel202P.DcDcBoost33Channel -> Adg715C.DcDcBoost33Channel;
	HplAccel202P.ChannelAccelPower -> Adg715C.ChannelAccelPower;
	HplAccel202P.ChannelAccel_X -> Adg715C.ChannelAccel_X;
	HplAccel202P.ChannelAccel_Y -> Adg715C.ChannelAccel_Y;
	HplAccel202P.Resource -> Adg715C.Resource[ unique(UQ_ADG715)];
}


