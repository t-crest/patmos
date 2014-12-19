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
 * A progress monitor for downloading
 *
 * Authors: Tórur Biskopstø Strøm (torur.strom@gmail.com)
 *
 */

package patserdow;

import java.io.PrintStream;

public class ProgressMonitor 
{
	final private int granularity = 10;
	private int byte_count = 0;
	private int byte_number;
	private PrintStream stream;
	
	public ProgressMonitor(int byte_number, PrintStream stream) 
	{
		this.byte_number = byte_number;
		this.stream = stream;
	}
	
	void update(int byte_count_increment)
	{
		this.byte_count += byte_count_increment;
		stream.print("\r[");
		int progress = (byte_count*granularity)/byte_number;
		for (int i = 0; i < progress; i++) 
		{
			stream.print('+');
		}
		for (int i = 0; i < granularity-progress; i++) 
		{
			stream.print(' ');
		}
		stream.print("] ");
		stream.print(byte_count);
		stream.print('/');
		stream.print(byte_number);
		stream.print(" bytes");
	}

}
