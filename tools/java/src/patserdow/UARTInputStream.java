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
 * Stream for reading input from UART
 *
 * Authors: Torur Biskopsto Strom (torur.strom@gmail.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patserdow;

import java.io.IOException;
import java.io.InputStream;

import jssc.SerialPort;
import jssc.SerialPortException;

public class UARTInputStream extends InputStream
{

	private SerialPort port;
	
	public UARTInputStream(SerialPort port) 
	{
		this.port = port;
	}
	
	@Override
	public long skip(long n) throws IOException 
	{
		try 
		{
			for (long i = 0; i < n; i++) {
				port.readBytes(1);
			}
			return 0;
		} 
		catch (SerialPortException e) 
		{
			throw new IOException(e);
		}
	}
	
	@Override
	public int available() throws IOException 
	{
		try 
		{
			return port.getInputBufferBytesCount();
		} 
		catch (SerialPortException e) 
		{
			throw new IOException(e);
		}
	}
	
	
	
	@Override
	public int read() throws IOException 
	{
		try 
		{
			int[] temp = port.readIntArray(1);
			if(temp == null || temp.length < 1)
			{
				return -1;
			}
			
			return temp[0];
		} 
		catch (SerialPortException e) 
		{
			throw new IOException(e);
		}
	}

}
