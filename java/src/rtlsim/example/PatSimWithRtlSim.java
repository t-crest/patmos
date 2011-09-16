package rtlsim.example;

import rtlsim.*;

public class PatSimWithRtlSim {

	/**
	 * @param args
	 */
	public static void main(String[] args) {

		PCWire nextPC = new PCWire();
		rtlsim.Register pc = new rtlsim.Register(nextPC);
		Fetch fe = new Fetch(pc, nextPC, new Instruction());

		Simulator.getInstance().simulate(10);
	}
}

class PCWire extends Wire {
	int val;
}

class Instruction extends Wire {
	int ia, ib;
}


class Fetch extends Logic {

	rtlsim.Register pcReg;
	PCWire nextPC;
	Instruction instr;
	
	int mem[] = {
			0xabcd000, 0x12345678,
			0x0000022, 0x00000033,
	};
	
	public Fetch(rtlsim.Register pc, PCWire nextPC, Instruction instr) {
		this.pcReg = pc;
		this.nextPC = nextPC;
		this.instr = instr;
	}
	
	@Override
	protected void calculate() {
		int pc = ((PCWire) pcReg.getVal()).val;
		nextPC.val = pc + 1;
		instr.ia = mem[pc];
		instr.ib = mem[pc+1];
		
		System.out.println(instr.ia);
	}
	
}

class TBOut extends Wire {
	
}

class TBDriver extends Logic {

	TBOut out;
	
	public TBDriver(TBOut out) {
		this.out = out;
	}
	@Override
	protected void calculate() {
		// We drive the simulation from this logic module
		
	}
	
}
