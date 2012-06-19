package rtlsim.example;

import rtlsim.*;

public class Mac {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		
		IntWire sum = new IntWire();
		Register accu = new Register(sum);
		IntWire bin = new IntWire();
		bin.val = 1;	// that's a constant wire
		new Adder(accu, bin, sum);
		
		Simulator.getInstance().simulate(10);
		
	}

}

class IntWire extends Port {
	int val;
}

class Adder extends Logic {
	
	Register a;
	IntWire b, c;
	
	public Adder(Register a, IntWire b, IntWire c) {
		this.a = a;
		this.b = b;
		this.c = c;
	}
	
	protected void calculate() {
		c.val = ((IntWire) a.getOutPort()).val + b.val;
		System.out.println(c.val);
	}
}