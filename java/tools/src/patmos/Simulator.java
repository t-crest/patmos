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

import java.util.ArrayList;

/**
 * Simulator for RTL simulation
 * 
 * @author martin
 *
 */
public class Simulator {
	
	private static Simulator single = new Simulator();
	ArrayList<Module> modules = new ArrayList<Module>();
	
	public static Simulator getInstance() {
		return single;
	}

	public void register(Module m) {
		modules.add(m);
	}
	
	public void simulate(int cnt) {
		for (int i=0; i<cnt; ++i) {
			for (Module ie : modules) {
				ie.calculate();
			}
			for (Module ie : modules) {
				ie.update();
			}
			for (Module ie : modules) {
				System.out.print(ie.toString());
			}
			System.out.println();
		}
	}

}
