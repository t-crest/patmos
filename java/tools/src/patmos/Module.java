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
package patmos;

import java.lang.reflect.Field;

/**
 * A module executed by the simulator.
 * @author martin
 *
 */
public abstract class Module {

	class State implements Cloneable {
		
		protected State clone() throws CloneNotSupportedException {
			// System.out.println("clone: "+this.getClass());
			return (State) super.clone();
		}
		
// does not really work, don't know why I don't get the fields in f.
//		public String toString(){
//			String s = "";
//			Class<? extends State> cl = getClass();
//			Field f[] = cl.getFields();
//			System.out.println("in toString of "+ cl + " "+ f+" len "+f.length);
//			for (int i = 0; i < f.length; i++) {
//				Field field = f[i];
//				s += field.getName() + " ";
//				// s += field.get
//				System.out.println(s);
//			}
//			return s;
//		}
	}
	
	State current, register;
	
	
	public Module() {
		Simulator.getInstance().register(this);
		current = new State();
		register = new State();
	}
	
	/**
	 * Execute combinational part
	 */
	abstract void calculate();
	
	/**
	 * Update the state (register);
	 */
	void update() {
		try {
			register = current.clone();
		} catch (CloneNotSupportedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	State getValue() {
		return register;
	}
	
	public String toString() {
		return register.toString();
	}
}
