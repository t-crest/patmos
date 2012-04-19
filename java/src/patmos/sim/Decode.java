/*
   Copyright 2011 Martin Schoeberl <masca@imm.dtu.dk>,
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

package patmos.sim;

/**
 * @author martin
 *
 */
public class Decode extends Module {
	
	class State extends Module.State {
		
		class DecState extends Module.State {
			int regA, regB;
		}
		
		boolean dual;
		DecState pipe[] = { new DecState(), new DecState() };
		
		int abc;		
		
		public String toString() {
			return "abc: "+abc+" ";
		}
		
		protected State clone() throws CloneNotSupportedException {
			// System.out.println("clone: "+this.getClass());
			State s = (State) super.clone();
			s.pipe[0] = (DecState) s.pipe[0].clone();
			s.pipe[1] = (DecState) s.pipe[1].clone();
			return s;
		}


	}
	
	int regFile[] = new int[32];
	Fetch feRef;

	public Decode() {
		current = new State();
		register = new State();
	}
	
	public void setFetchRef(Fetch fe) {
		feRef = fe;
	}

	/* (non-Javadoc)
	 * @see patmos.Module#calculate()
	 */
	@Override
	void calculate() {
		State s = (State) current;
		Fetch.State fs = (Fetch.State) feRef.getValue();
		s.dual = fs.dual;
		for (int i=0; i<2; ++i) {
			s.pipe[i].regA = regFile[(fs.instr[i]>>(Const.REG_SHIFT+5)) & 0x1f];
			s.pipe[i].regB = regFile[(fs.instr[i]>>(Const.REG_SHIFT)) & 0x1f];			
		}
		
		s.abc = fs.pc+100;
	}

}
