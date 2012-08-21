/*
   Copyright 2012 Martin Schoeberl <masca@imm.dtu.dk>,
                  Technical University of Denmark, DTU Informatics. 
   All rights reserved.

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

package util;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.util.LinkedList;
import java.util.List;
import java.util.Scanner;

/**
 * Small tool to compare high-level and VHDL simulation results. Read in the
 * register dumps and compare.
 * 
 * @author martin
 * 
 */
public class CompTest {

	/**
	 * @param args
	 */
	public static void main(String[] args) throws Exception {

		if (args.length != 2) {
			System.out.println("usage: java CompTest hs-log ms-log");
			System.exit(-1);
		}

		// this is dumb code, but to lazy to write something more elaborate
		Scanner hs = new Scanner(new FileInputStream(args[0]));
		Scanner ms = new Scanner(new FileInputStream(args[1]));

		// Read till ModelSim useful output
		while (ms.hasNextLine()) {
			String s = ms.nextLine();
			if (s.startsWith("# Patmos start")) {
				break;
			}
		}
		// Drop 9 clock cycles for reset...
		for (int i = 0; i < 9; ++i) {
			ms.nextLine();
		}

		// Drop first 5 cycles form high level simulation
		for (int i = 0; i < 5; ++i) {
			hs.nextLine();
		}

		// Now we should be synchronous
		while (hs.hasNextLine()) {
			System.out.println(hs.nextLine());
			System.out.println(ms.nextLine());
		}

	}

}
