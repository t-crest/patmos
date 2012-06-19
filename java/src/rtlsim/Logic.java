package rtlsim;

public abstract class Logic {
	
	public Logic() {
		Simulator.getInstance().register(this);
	}
	protected abstract void calculate();

}
