package rtlsim;

public class Wire implements Cloneable {

	protected Wire clone() {
		Wire w = null;
		try {
			w = (Wire) super.clone();
		} catch (CloneNotSupportedException e) {
			e.printStackTrace();
		}
		return w;
	}
}
