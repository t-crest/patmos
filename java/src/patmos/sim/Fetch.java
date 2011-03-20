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
public class Fetch extends Module {
	
	class State extends Module.State {
		int pc;
		int instr[] = new int[2];
		boolean dual;
		
		public String toString() {
			return "pc: "+pc+" instr: "+instr[0]+" "+instr[1]+" ";
		}
		
		protected State clone() throws CloneNotSupportedException {
			// System.out.println("clone: "+this.getClass());
			State s = (State) super.clone();
			s.instr = instr.clone();
			return s;
		}
	}
	
	int mcache[] = {
			1, 2,
			0x80000000+3, 4,
			555, 6666,
			7, 8, 9, 10, 11, 12, 13, 14, 15
	};

	public Fetch() {
		current = new State();
		register = new State();
	}
	/* (non-Javadoc)
	 * @see patmos.Module#calculate()
	 */
	@Override
	void calculate() {
		State s = (State) current;
		s.instr[0] = mcache[s.pc];
		s.instr[1] = mcache[s.pc+1];
		s.dual = (s.instr[0] & 0x80000000) != 0;
		s.pc++;
		if (s.dual) {
			s.pc++;
		}
	}

}
