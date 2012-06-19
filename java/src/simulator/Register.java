package simulator;

public class Register {

	static class Data {
		int pc;
		static class Fetch {
			int instr[] = new int[2];
			boolean valid[] = new boolean[2];
		}
		Fetch fetch = new Fetch();
		static class Decode {
			int regA[] = new int[2];
			int regB[] = new int[2];
		}
		Decode decode = new Decode();
	}
	
	Data d = new Data();
	Data q = new Data();
	
	void tick() {
		q.pc = d.pc;
		for (int i=0; i<2; ++i) {
			q.fetch.instr[i] = d.fetch.instr[i];
			q.fetch.valid[i] = d.fetch.valid[i];
			q.decode.regA[i] = d.decode.regA[i];
			q.decode.regB[i] = d.decode.regB[i];
		}
	}

}
