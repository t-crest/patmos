package simulator;

public class SimPat {

	Register reg = new Register();
	
	int mem[] = {
			0xabcd0000, 0x12345678,
			0x00000022, 0x00000033,
			0x80000001, 0x00000002,
			0x00000003, 0x00000004,
	};
	
	public void tick() {
		reg.d.fede.instr[0] = mem[reg.q.pc];
		reg.d.fede.instr[1] = mem[reg.q.pc+1];
		reg.d.fede.valid[0] = true;
		if ((reg.d.fede.instr[0] & 0x80000000) != 0) {
			reg.d.pc = reg.q.pc+2;
			reg.d.fede.valid[0] = true;			
		} else {
			reg.d.pc = reg.q.pc+1;
			reg.d.fede.valid[0] = false;
			
		}
		
		reg.tick();
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {

		SimPat sim = new SimPat();
		for (int i=0; i<10; ++i) {
			System.out.println(sim.reg.q.pc + " " + sim.reg.q.fede.instr[0]);
			sim.tick();
		}
	}
}
