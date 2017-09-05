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

import java.io.FileInputStream;
import java.util.Scanner;
import java.util.regex.Pattern;

/**
 * Small tool to compare Chisel/c emulator results with the ISA
 * simulation of Patmos (PatSim). Read in the register dumps and compare.
 * 
 * @author Martin Schoeberl <masca@imm.dtu.dk>
 * 
 * TODO: compare tools should be merged - just scanning start, register dump,
 * and end need to be factored out
 * 
 */
public class CompareScala {

	/**
	 * @param args
	 */
	public static void main(String[] args) throws Exception {

		if (args.length != 2) {
			System.out
					.println("usage: java CompareScala PatSim-log chisel-log");
			System.exit(-1);
		}

		// this is dumb code, but to lazy to write something more elaborate
		Scanner hs = new Scanner(new FileInputStream(args[0]));
		Scanner chisel = new Scanner(new FileInputStream(args[1]));

		int line = 1;

		// Read till the Chisel C based simulation provides useful output
		while (chisel.hasNextLine()) {
			String s = chisel.nextLine();
			line++;
			if (s.startsWith("Patmos start")) {
				break;
			}
		}

		// Read until ISA sim provides useful output
		while (hs.hasNextLine()) {
			String s = hs.nextLine();
			line++;
			if (s.startsWith("Patmos start")) {
				break;
			}
		}


		if (!hs.hasNextLine()) {
			System.out.println("No suitable output from ISA simulator");
			System.exit(1);
		}

		if (!chisel.hasNextLine()) {
			System.out.println("No suitable output from chisel emulator");
			System.exit(1);
		}

		// This is now more tolerant related to different timings.
		// We compare only register dumps when at least one register has
		// changed.
		// However, this also might show the difference a little bit later.
		int hsReg[] = new int[32];
		int csReg[] = new int[32];
		int hsRegOld[] = new int[32];
		int csRegOld[] = new int[32];

		int cnt = 1;
		int pc = 0;
		Pattern makeExitPattern = Pattern.compile("make\\S*:");
		Pattern passedExitPattern = Pattern.compile("PASSED");
		
		int cntNoChange = 0;
		while (hs.hasNextLine() && chisel.hasNextLine()) {
			// workaround for exits with error code
			if (hs.hasNext(makeExitPattern)) {
				break;
			}
			if (chisel.hasNext(makeExitPattern)) {
				System.out.println("Chisel trace incomplete: " + cnt);
				System.exit(1);
			}
			while (true) {
				if (chisel.hasNext(passedExitPattern)) {
					break;
				}
				// unsigned int output from Chisel
				pc = (int) chisel.nextLong();
				// System.out.print("pc: "+pc);
				chisel.next(); // skip '-'
				boolean change = false;
				for (int i = 0; i < 32; ++i) {
					csReg[i] = (int) chisel.nextLong();
					if (csReg[i] != csRegOld[i]) {
						change = true;
						cntNoChange = 0;
					}
				}
				if (change) {
					for (int i = 0; i < 32; ++i) {
						csRegOld[i] = csReg[i];
					}
					break;
				} else {
					++cntNoChange;
					if (chisel.hasNextLine()) {
						chisel.nextLine();
						line++;
					} else {
						break;
					}
					if (cntNoChange>10000) {
						System.out.println("No change in Chisel simulation");
						System.exit(1);
					}
				}
			}
			cntNoChange = 0;
			while (true) {
				if (hs.hasNext(passedExitPattern)) {
					break;
				}
				// signed int output from Scala
				pc = (int) hs.nextInt();
				hs.next(); // skip '-'
				boolean change = false;
				for (int i = 0; i < 32; ++i) {
					// To skip register names and switch to hex see compare with pasim
					hsReg[i] = (int) hs.nextInt();
					if (hsReg[i] != hsRegOld[i]) {
						change = true;
						cntNoChange = 0;
					}
				}
				if (change) {
					for (int i = 0; i < 32; ++i) {
						hsRegOld[i] = hsReg[i];
					}
					break;
				} else {
					++cntNoChange;
					if (hs.hasNextLine()) {
						hs.nextLine();						
					} else {
						break;
					}
					if (cntNoChange>1000) {
						System.out.println("No change in ISA simulation");
						System.exit(1);
					}
				}
			}
			for (int i = 0; i < 32; ++i) {
				if (csReg[i] != hsReg[i]) {
					System.out.println("Difference at PC: " + pc + " line: " + line);
					System.out.println("Register " + i + " Chisel: " + csReg[i]
							+ " isasim: " + hsReg[i]);
					System.exit(1);
				}
			}
			if (!chisel.hasNext(passedExitPattern) && chisel.hasNextLine()) {
				chisel.nextLine();
				line++;
			} else {
				break;
			}
			if (hs.hasNextLine()) {
				hs.nextLine();						
			} else {
				break;
			}
			++cnt;
		}
		System.out.println(" ok");
		System.exit(0);
	}

}
