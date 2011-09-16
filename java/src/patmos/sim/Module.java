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
