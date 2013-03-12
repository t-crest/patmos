/*
   Copyright 2013 Technical University of Denmark, DTU Compute. 
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
import java.util.regex.Pattern;
/**
 * Small tool to compare high-level and Chisel/c simulation results. Read in the
 * register dumps and compare.
 * 
 * @author Martin Schoeberl <masca@imm.dtu.dk>
 * 
 */
public class CompareChisel {

	/**
	 * @param args
	 */
	public static void main(String[] args) throws Exception {

		if (args.length != 2) {
			System.out.println("usage: java CompareChisel pasim-log chisel-log");
			System.exit(-1);
		}

		// this is dumb code, but to lazy to write something more elaborate
		Scanner hs = new Scanner(new FileInputStream(args[0]));
		Scanner chisel = new Scanner(new FileInputStream(args[1]));

        // Read till the Chisel C based simulation prvodes useful output
		while (chisel.hasNextLine()) {
			String s = chisel.nextLine();
			if (s.startsWith("STARTING")) {
				break;
			}
		}
		// Drop 5 clock cycles for pipeline fill
		for (int i = 0; i < 5 && chisel.hasNextLine(); ++i) {
			chisel.nextLine();
		}

		// Drop first 4 cycles form high level simulation
		// we keep this first instruction executing thing
		for (int i = 0; i < 5 && hs.hasNextLine(); ++i) {
			hs.nextLine();
		}
		
		// TODO we should be more tolerant on different timings (stall cycles)

		if (!hs.hasNextLine()) {
			System.out.println("No suitable output from high-level simulator");
			System.exit(1);
		}

		if (!chisel.hasNextLine()) {
			System.out.println("No suitable output from high-level simulator");
			System.exit(1);
		}

		int cnt = 1;
		// Now we should be synchronous
		Pattern makeExitPattern = Pattern.compile("make\\S*:");
		while (hs.hasNextLine()) {
			// workaround for exist with error code
			if (hs.hasNext(makeExitPattern)) {
				break;
			}
			if (chisel.hasNext(makeExitPattern)) {
				System.out.println("Chisel trace incomplete: "+cnt);
				System.exit(1);
			}
			// unsigned int output from Chisel
			int pc = (int) chisel.nextLong();
			chisel.next(); // skip '-'
			// TODO: we would like to keep the ':' after the pc, but I don't have a Scanner docu
			for (int i=0; i<32; ++i) {
				int csVal = (int) chisel.nextLong();
				while (!hs.hasNextLong(16)) {
					hs.next();
				}
				int hsVal = (int) hs.nextLong(16);
				if (csVal != hsVal) {
					System.out.println("Difference at PC: "+pc);
					System.out.println("Register "+i+ " Chisel: "+csVal+" patsim: "+hsVal);
					System.exit(1);
				}
			}

			hs.nextLine();
			chisel.nextLine();
			++cnt;
		}
		System.out.println(" ok");
		System.exit(0);
	}

}
