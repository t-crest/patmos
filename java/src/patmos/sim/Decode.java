/*
  This file is part of JOP, the Java Optimized Processor
    see <http://www.jopdesign.com/>

  Copyright (C) 2010, Martin Schoeberl (martin@jopdesign.com)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
