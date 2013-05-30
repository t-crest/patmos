package patserdow;

import java.io.PrintStream;

public class ProgressMonitor 
{
	final private int granularity = 10;
	private int byte_count = 0;
	private int byte_number;
	private PrintStream stream;
	
	public ProgressMonitor(int byte_number, PrintStream stream) 
	{
		this.byte_number = byte_number;
		this.stream = stream;
	}
	
	void update(int byte_count_increment)
	{
		this.byte_count += byte_count_increment;
		stream.print("\r[");
		int progress = (byte_count*granularity)/byte_number;
		for (int i = 0; i < progress; i++) 
		{
			stream.print('+');
		}
		for (int i = 0; i < granularity-progress; i++) 
		{
			stream.print(' ');
		}
		stream.print("] ");
		stream.print(byte_count);
		stream.print('/');
		stream.print(byte_number);
		stream.print(" bytes");
	}

}
