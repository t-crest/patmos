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
	final private int FRAME_SIZE = 255; //Must fit into a byte
	final private int FRAME_SIZE_OFFSET = 1;
	final private int CRC_SIZE = 4;
	
	public Transmitter(InputStream instream, OutputStream outstream)
	{
		INSTREAM = instream;
		OUTSTREAM = outstream;
	}
	
	private void send(byte[] buffer, int offset, int length) throws IOException, InterruptedException 
	{
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
		int response = INSTREAM.read();
		if( response != 'o')
		{
			throw new java.io.IOException("Receiver did not reply correctly["+response+"]!(CRC might be incorrect)");
		}
	}
	
	void send(InputStream stream, int length, ProgressMonitor monitor) throws IOException, InterruptedException
	{
		CRC32 CRC = new CRC32();
		byte[] buffer = new byte[FRAME_SIZE_OFFSET+FRAME_SIZE+CRC_SIZE];
        int remaining = length;
		while(remaining > 0) 
		{
    		CRC.reset();
    		int size = FRAME_SIZE;
    		if(remaining < FRAME_SIZE)
    		{
    			//Some streams will read 0 after EOF
    			size = remaining;
    		}
    		int read = stream.read(buffer,FRAME_SIZE_OFFSET,size);
    		buffer[0] = (byte)read;
    		CRC.update(buffer,FRAME_SIZE_OFFSET,read);
    		ByteBuffer byteBuffer = ByteBuffer.wrap(buffer,FRAME_SIZE_OFFSET+read,CRC_SIZE);
            byteBuffer.putInt((int)CRC.getValue());
            send(buffer,0,FRAME_SIZE_OFFSET+read+CRC_SIZE);
            remaining -= read;
            monitor.update(read);
		}
	}
}