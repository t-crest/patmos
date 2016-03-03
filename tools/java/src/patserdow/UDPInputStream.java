/*
   Copyright 2016 Technical University of Denmark, DTU Compute.
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
 * Stream for reading input via UDP
 *
 * Authors: Torur Biskopsto Strom (torur.strom@gmail.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patserdow;

import java.io.IOException;
import java.io.InputStream;

import java.net.DatagramSocket;
import java.net.DatagramPacket;

public class UDPInputStream extends InputStream {
	private DatagramSocket socket;
    private DatagramPacket packet;
    private int pos;

	public UDPInputStream(DatagramSocket socket) {
		this.socket = socket;
        byte[] buf = new byte[0x600];
        packet = new DatagramPacket(buf, buf.length);
        pos = packet.getLength();
	}
	
	@Override
	public long skip(long n) throws IOException {
        for (long i = 0; i < n; i++) {
            read();
        }
        return 0;
	}
	
	@Override
	public int available() throws IOException {
        return packet.getLength()-pos;
	}	
	
	@Override
	public int read() throws IOException {
        while (pos >= packet.getLength()) {
            socket.receive(packet);
            pos = 0;
        }
        return packet.getData()[pos++] & 0xff;
	}
}
