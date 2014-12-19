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
 * Stream for writing output to UART
 *
 * Authors: Tórur Biskopstø Strøm (torur.strom@gmail.com)
 *
 */

package patserdow;

import java.io.IOException;
import java.io.OutputStream;

import jssc.SerialPort;
import jssc.SerialPortException;

public class UARTOutputStream extends OutputStream
{

	private SerialPort port;
	
	public UARTOutputStream(SerialPort port) 
	{
		this.port = port;
	}
	
	@Override
	public void write(byte[] b, int off, int len) throws IOException 
	{
		if(off > 0 || len < b.length)
		{
			byte[] newbuffer = new byte[len-off]; //Because SerialPort doesn't do offset
			System.arraycopy(b, off, newbuffer, 0, len);
			b = newbuffer;
		}
		try 
		{
			port.writeBytes(b);
		} 
		catch (SerialPortException e) 
		{
			throw new IOException(e);
		}
	}
	
	@Override
	public void write(int b) throws IOException 
	{
		try 
		{
			port.writeByte((byte)b);
		} 
		catch (SerialPortException e) 
		{
			throw new IOException(e);
		}
	}

}
