package rtlsim;

import java.lang.reflect.Field;

public class Register implements Cloneable {

	Wire d, q;

	public Register(Wire o) {
		Simulator.getInstance().register(this);
		d = o;
		q = o;
	}

	void tick() {
		q = d.clone();
	}

	public Wire getVal() {
		return q;
	}

	public String toString() {
		String s = "";
		Class<? extends Register> cl = getClass();
		Field f[] = cl.getFields();
		System.out.println("in toString of " + cl + " " + f + " len "
				+ f.length);
		for (int i = 0; i < f.length; i++) {
			Field field = f[i];
			s += field.getName() + " ";
			// s += field.get
			System.out.println(s);
		}
		return s;
	}

}
