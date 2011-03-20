/*
--
--  Copyright 2011 Martin Schoeberl <masca@imm.dtu.dk>,
--                 Technical University of Denmark, DTU Informatics. 
--  All rights reserved.
--
--  License: TBD, BSD style requested, decision pending.
--
 */

package patmos.asm;

import java.util.HashMap;
import java.util.Map;
import java.util.TreeMap;

public class Instruction {

	public String name;
	public int opcode;
	public int opdSize;
	public Type type;
	
	enum Type {
		ALU, BRANCH, INDLDST, IO, NOP
	};

	public Instruction(String s, int opc, int ops, Type t) {
		name = s;
		opcode = opc;
		opdSize = ops;
		type = t;
	}

	private static Instruction[] ia = new Instruction[] {
		new Instruction("nop",   0x0000, 0, Type.NOP),
		new Instruction("add",   0x0800, 8, Type.ALU),
		new Instruction("sub",   0x0c00, 8, Type.ALU),
		new Instruction("shr",   0x1000, 8, Type.ALU),
		new Instruction("load",  0x2000, 8, Type.ALU),
		new Instruction("and",   0x2200, 8, Type.ALU),
		new Instruction("or",    0x2400, 8, Type.ALU),
		new Instruction("xor",   0x2600, 8, Type.ALU),
		new Instruction("loadh", 0x2800, 8, Type.ALU),
		new Instruction("store", 0x3000, 8, Type.NOP),
		new Instruction("out",   0x3800, 8, Type.IO),
		new Instruction("in",    0x3c00, 8, Type.IO),
		new Instruction("brnz",  0x4800, 8, Type.BRANCH),
	};

	public static Map<String, Instruction> map = new HashMap<String, Instruction>();
	public static Map<Integer, Instruction> imap = new TreeMap<Integer, Instruction>();

	static {
		for (int i = 0; i < ia.length; ++i) {
			map.put(ia[i].name, ia[i]);
			imap.put(ia[i].opcode, ia[i]);
		}
	}

	public static Instruction get(String name) {
		return map.get(name);
	}

	public static Instruction get(int opcode) {
		return imap.get(opcode);
	}

	/**
	 * Return immediate encoding if supported.
	 * @return
	 */
	public int setImmediate() {
		int op = opcode;
		if (opdSize==0) {
			throw new Error("Immediate not supported for "+this);			
		}
		if (type==Type.ALU) {
			op |= 0x0100;
		}
		return op;
	}

	public String toString() {
		return name;
	}
}
