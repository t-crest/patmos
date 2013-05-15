package patserdow;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.zip.CRC32;

import jssc.SerialPort;
import jssc.SerialPortException;
import jssc.SerialPortTimeoutException;

public class Transmitter
{

	final private SerialPort port;
	final private int FRAME_SIZE = 200;
	final private int SIZE_OFFSET = 1;
	final private int CRC_SIZE = 4;
	final private int BAUD_RATE = 115200;
	final private byte BUFFER[] = new byte[SIZE_OFFSET+FRAME_SIZE+CRC_SIZE];
	final byte[] MAGIC_FRAME = {(byte)0x01,(byte)0xAB,(byte)0x93,(byte)0x06,(byte)0x95,(byte)0xED};
	final CRC32 CRC = new CRC32();
	
	public Transmitter(String portName) throws SerialPortException
	{
		port = new SerialPort(portName);
		System.out.println("Port opened: " + port.openPort());
        System.out.println("Params set: " + port.setParams(BAUD_RATE, 8, 1, 0));
        
	}
	
	private void send(byte[] buffer) throws SerialPortException, SerialPortTimeoutException, InterruptedException, IOException
	{
		port.readString(); //Empty input buffer
		int retries = 10;
        while(true)
        {
        	port.writeBytes(buffer);
    		//Events do not seem to work so doing it manually
    		long timeout = System.currentTimeMillis();
    		while(true)
    		{
    			if(port.getInputBufferBytesCount() > 0)
    			{
    				break;
    			}
    			if(System.currentTimeMillis() - timeout > 1000)
    			{
    				throw new java.io.IOException("Timeout!");
    			}
    			Thread.sleep(1);
    		}
    		String response = port.readString();
    		//System.out.println(response);
    		if(response.equals("o"))
    		{
    			break;
    		}
			if(retries == 0)
	        {
	        	throw new java.io.IOException("Receiver keeps asking for resends!(CRC might be incorrect)");
	        }
			retries--;
        }
	}
	
	boolean send(InputStream stream) throws IOException, SerialPortException, InterruptedException, SerialPortTimeoutException
	{
        try 
        {
        	int totalsize = stream.available();
        	while(stream.available() > 0)
        	{
        		
        		System.out.println("%"+((totalsize-stream.available())*100/totalsize));
        		CRC.reset();
        		int length = stream.read(BUFFER, SIZE_OFFSET, FRAME_SIZE);
        		byte buffer[] = BUFFER;
        		if(length < FRAME_SIZE)
        		{
        			//SerialPort is missing offset so create smaller buffer
        			buffer = new byte[SIZE_OFFSET+length+CRC_SIZE];
        		}
        		buffer[0] = (byte)length;
        		CRC.update(buffer, SIZE_OFFSET, length);
        		ByteBuffer byteBuffer = ByteBuffer.wrap(buffer, SIZE_OFFSET+length, CRC_SIZE);
                byteBuffer.putInt((int)CRC.getValue());
                send(buffer);
        	}
        	send(MAGIC_FRAME);
        }
        finally
        {
            if(port != null)
            {
            	port.closePort();
            }
        }
		return true;
	}
}
