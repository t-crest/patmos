package rtlsim.example;

import rtlsim.*;

public class PatSimWithRtlSim {

	final static int REG_SHIFT = 7;

	/**
	 * @param args
	 */
	public static void main(String[] args) {

		Register pc = new Register(new PcPort());
		Register fedec = new Register(new FeDePort());
		Register deex = new Register(new DeExPort());
		
		Fetch fe = new Fetch(pc, fedec);
		Decode de = new Decode(fedec, deex);

		Simulator.getInstance().simulate(6);
	}
}

class PcPort extends Port {
	int val;
}

class FeDePort extends Port {
	int instr[] = new int[2];
	boolean valid[] = new boolean[2];
	int pc;
	public FeDePort clone() {
		FeDePort v = (FeDePort) super.clone();
		v.instr = v.instr.clone();
		v.valid = v.valid.clone();
		return v;

	}
}

class DeExPort extends Port {
	
	static class DecOut {
		int regA, regB;	
	}
	DecOut decout[] = {new DecOut(), new DecOut()};
	public DeExPort clone() {
		DeExPort v = (DeExPort) super.clone();
		v.decout = v.decout.clone();
		return v;
	}
}

class Fetch extends Logic {

	Register pc;
	Register fedec;
	
	int mem[] = {
			0xabcd0000, 0x12345678,
			0x00000022, 0x00000033,
			0x80000001, 0x00000002,
			0x00000003, 0x00000004,
			0,0,0,0,0,0,0,
	};
	
	public Fetch(Register pc, Register fedec) {
		this.pc = pc;
		this.fedec = fedec;
	}
	
	@Override
	protected void calculate() {
		
		PcPort pcOut = (PcPort) pc.getOutPort();
		PcPort pcIn = (PcPort) pc.getInPort();
		FeDePort instrIn = (FeDePort) fedec.getInPort();

		instrIn.instr[0] = mem[pcOut.val];
		instrIn.instr[1] = mem[pcOut.val+1];
		instrIn.pc = pcOut.val;
		instrIn.valid[0] = true;
		if ((instrIn.instr[0] & 0x80000000) != 0) {
			instrIn.valid[1] = true;
			pcIn.val = pcOut.val+2;
		} else {
			instrIn.valid[1] = false;			
			pcIn.val = pcOut.val+1;
		}
		
		
		System.out.println("fetch: "+instrIn.valid[1] + " " + instrIn.instr[0]);
	}
	
}

class Decode extends Logic {
	
	Register fedec, deex;

	public Decode(Register fedec, Register deex) {
		this.fedec = fedec;
		this.deex = deex;
	}

	@Override
	protected void calculate() {
		
		FeDePort decIn = (FeDePort) fedec.getOutPort();
		DeExPort decOut = (DeExPort) deex.getInPort();
		
		for (int i=0; i<2; ++i) {
			int instr = decIn.instr[i];
			// read register addresses
			decOut.decout[i].regA = (instr>>(PatSimWithRtlSim.REG_SHIFT+5)) & 0x1f;
			decOut.decout[i].regB = (instr>>(PatSimWithRtlSim.REG_SHIFT)) & 0x1f;
			
		}

		System.out.println("decode: " + decIn.valid[1] + " "+decOut.decout[0].regA);
		
	}
	
}
class TBOut extends Port {
	
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
