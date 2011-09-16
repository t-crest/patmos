package simulator;

public class SimPat {
	
	final static int REG_SHIFT = 0; // 7;

	Register reg = new Register();
	
	int mem[] = {
			0xabcd0000, 0x12345678,
			0x00000022, 0x00000033,
			0x80000001, 0x00000002,
			0x00000003, 0x00000004,
	};
	
	public void tick() {
		
		// fetch and PC stage
		reg.d.fetch.instr[0] = mem[reg.q.pc];
		reg.d.fetch.instr[1] = mem[reg.q.pc+1];
		reg.d.fetch.valid[0] = true;
		if ((reg.d.fetch.instr[0] & 0x80000000) != 0) {
			reg.d.pc = reg.q.pc+2;
			reg.d.fetch.valid[1] = true;			
		} else {
			reg.d.pc = reg.q.pc+1;
			reg.d.fetch.valid[1] = false;
			
		}
		
		// decode stage
		for (int i=0; i<2; ++i) {
			int instr = reg.q.fetch.instr[i];
			// read register addresses
			reg.d.decode.regA[i] = (instr>>(REG_SHIFT+5)) & 0x1f;
			reg.d.decode.regB[i] = (instr>>(REG_SHIFT)) & 0x1f;
			
		}

		
		reg.tick();
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {

		SimPat sim = new SimPat();
		for (int i=0; i<10; ++i) {
			System.out.println(sim.reg.q.pc + " " + sim.reg.q.fetch.instr[0]
			                   + " " + sim.reg.q.decode.regB[0]);
			sim.tick();
		}
	}
}
