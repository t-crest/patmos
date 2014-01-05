package patserdow;

import java.io.IOException;
import java.io.InputStream;

import jssc.SerialPort;
import jssc.SerialPortException;

public class UARTInputStream extends InputStream
{

	private SerialPort port;
	
	public UARTInputStream(SerialPort port) 
	{
		this.port = port;
	}
	
	@Override
	public long skip(long n) throws IOException 
	{
		try 
		{
			port.readString();
			return 0;
		} 
		catch (SerialPortException e) 
		{
			throw new IOException(e);
		}
	}
	
	@Override
	public int available() throws IOException 
	{
		try 
		{
			return port.getInputBufferBytesCount();
		} 
		catch (SerialPortException e) 
		{
			throw new IOException(e);
		}
	}
	
	
	
	@Override
	public int read() throws IOException 
	{
		try 
		{
			int[] temp = port.readIntArray(1);
			if(temp == null || temp.length < 1)
			{
				return -1;
			}
			
			return temp[0];
		} 
		catch (SerialPortException e) 
		{
			throw new IOException(e);
		}
	}

}
