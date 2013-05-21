package patserdow;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.zip.CRC32;

public class Transmitter
{

	//final private SerialPort port;
	final private InputStream INSTREAM;
	final private OutputStream OUTSTREAM;
	final private int FRAME_SIZE = 255;
	final private int SIZE_OFFSET = 1;
	final private int CRC_SIZE = 4;
	final private byte BUFFER[] = new byte[SIZE_OFFSET+FRAME_SIZE+CRC_SIZE];
	final byte[] MAGIC_FRAME = {(byte)0x01,(byte)0xAB,(byte)0x93,(byte)0x06,(byte)0x95,(byte)0xED};
	final CRC32 CRC = new CRC32();
	
	public Transmitter(InputStream instream, OutputStream outstream)
	{
		INSTREAM = instream;
		OUTSTREAM = outstream;
	}
	
	private void send(byte[] buffer, int offset, int length) throws IOException, InterruptedException 
	{
		//INSTREAM.skip(10000);
		//port.readString(); //Empty input buffer
		int retries = 4;
        while(true)
        {
        	System.err.println("length:"+length);
        	OUTSTREAM.write(buffer, offset, length);
    		//Events do not seem to work so doing it manually
    		long timeout = System.currentTimeMillis();
    		while(true)
    		{
    			if(INSTREAM.available() > 0)
    			{
    				break;
    			}
    			if(System.currentTimeMillis() - timeout > 1000)
    			{
    				throw new java.io.IOException("Timeout!");
    			}
    			Thread.sleep(1);
    		}
    		char response = (char)INSTREAM.read();
    		System.err.println(response);
    		if(response == 'o')
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
	
	boolean send(InputStream inputstream) throws IOException, InterruptedException
	{
    	int totalsize = inputstream.available();
    	while(inputstream.available() > 0)
    	{
    		
    		System.err.println("%"+((totalsize-inputstream.available())*100/totalsize));
    		CRC.reset();
    		int length = inputstream.read(BUFFER,SIZE_OFFSET,FRAME_SIZE);
    		BUFFER[0] = (byte)length;
    		CRC.update(BUFFER,SIZE_OFFSET,length);
    		ByteBuffer byteBuffer = ByteBuffer.wrap(BUFFER,SIZE_OFFSET+length,CRC_SIZE);
            byteBuffer.putInt((int)CRC.getValue());
            send(BUFFER,0,SIZE_OFFSET+length+CRC_SIZE);
    	}
    	send(MAGIC_FRAME,0,MAGIC_FRAME.length);
		return true;
	}
}
