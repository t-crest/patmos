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
public class Execute extends Module {

	class State extends Module.State {
		int xyz;		
		int aaa;
		
		public String toString() {
			return "xyz: "+xyz+" ";
		}

	}
	
	Decode deRef;

	public Execute() {
		current = new State();
		register = new State();
	}
	
	public void setDecodeRef(Decode de) {
		deRef = de;
	}

	/* (non-Javadoc)
	 * @see patmos.Module#calculate()
	 */
	@Override
	void calculate() {
		State s = (State) current;
		Decode.State ds = (Decode.State) deRef.getValue();
		s.xyz = ds.abc+300;
	}

}
