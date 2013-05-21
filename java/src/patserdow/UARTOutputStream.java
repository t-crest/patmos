package patserdow;

import java.io.IOException;
import java.io.OutputStream;

import jssc.SerialPort;
import jssc.SerialPortException;

public class UARTOutputStream extends OutputStream
{

	private SerialPort port;
	
	public UARTOutputStream(SerialPort port) 
	{
		this.port = port;
	}
	
	@Override
	public void write(byte[] b, int off, int len) throws IOException 
	{
		byte[] newbuffer = new byte[len]; //Because SerialPort doesn't do offset
		System.arraycopy(b, off, newbuffer, 0, len);
		try 
		{
			port.writeBytes(newbuffer);
		} 
		catch (SerialPortException e) 
		{
			throw new IOException(e);
		}
	}
	
	@Override
	public void write(int b) throws IOException 
	{
		try 
		{
			port.writeByte((byte)b);
		} 
		catch (SerialPortException e) 
		{
			throw new IOException(e);
		}
	}

}
