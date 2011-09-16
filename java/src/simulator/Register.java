package simulator;

public class Register {

	static class Data {
		int pc;
		static class FeDe {
			int instr[] = new int[2];
			boolean valid[] = new boolean[2];
		}
		FeDe fede = new FeDe();
	}
	
	Data d = new Data();
	Data q = new Data();
	
	void tick() {
		q.pc = d.pc;
		for (int i=0; i<2; ++i) {
			q.fede.instr[i] = d.fede.instr[i];
			q.fede.valid[i] = q.fede.valid[i];
		}
	}

}
