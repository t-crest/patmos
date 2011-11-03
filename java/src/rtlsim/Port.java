package rtlsim;

public class Port implements Cloneable {

	protected Port clone() {
		Port w = null;
		try {
			w = (Port) super.clone();
		} catch (CloneNotSupportedException e) {
			e.printStackTrace();
		}
		return w;
	}
}
