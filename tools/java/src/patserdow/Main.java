package patserdow;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.SequenceInputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.concurrent.TimeoutException;

import nl.lxtreme.binutils.elf.Elf;
import nl.lxtreme.binutils.elf.ElfHeader;
import nl.lxtreme.binutils.elf.ProgramHeader;
import jssc.*;

public class Main 
{
	final private static int BAUD_RATE = 115200;
	
    /**
     * @param args the command line arguments
     * @throws TimeoutException 
     * @throws SerialPortTimeoutException 
     */
    public static void main(String[] args) throws IOException, InterruptedException, SerialPortException, TimeoutException, SerialPortTimeoutException
    {
        boolean verbose = true;
        boolean error = false;

        PrintStream print_stream = System.err;
        InputStream in_stream = null;
        OutputStream out_stream = null;
        
        SerialPort port = null;
        try 
        {
            verbose = System.getProperty("verbose", "false").equals("true");

        	File file = null;
        	switch(args.length)
            {
                case 2:
                    port = new SerialPort(args[0]);
                    if (verbose) {
                        print_stream.println("Port opened: " + port.openPort());
                        print_stream.println("Params set: " + port.setParams(BAUD_RATE, 8, 2, 0));
                    } else {
                        port.openPort();
                        port.setParams(BAUD_RATE, 8, 2, 0);
				    }
                    in_stream = new UARTInputStream(port);
                    out_stream = new UARTOutputStream(port);
                    file = new File(args[1]);
                    break;
                case 1:
                	in_stream = System.in;
                    out_stream = System.out;
                    file = new File(args[0]);
                    break;
                default:
                	throw new IllegalArgumentException("Usage: patserdow [COMPORT] FILENAME");
            }
        	
            Elf elf = new Elf(file);
            ElfHeader header = elf.getHeader();
            if (verbose) {
                print_stream.println("Elf version is '1': "+(header.getVersion()==1));
                print_stream.println("CPU type is: "+header.getMachineType());
                print_stream.println("Instruction width is 32 bits: "+(header.is32bit()));
                print_stream.println("Is Big Endian: "+header.isBigEndian());
                print_stream.println("File is of type exe: "+(header.getType()==ElfHeader.ET_EXEC));
                print_stream.println("Entry point: "+header.getEntryPoint());
                print_stream.println();
            }

            Transmitter transmitter = new Transmitter(in_stream,out_stream);
            //Transmitter transmitter = new Transmitter(System.in, stream);

            final int HEADER_SIZE = 8;
            final int SEGMENT_HEADER_SIZE = 12;

            ProgramHeader [] segments = elf.getProgramHeaders();
            int byte_count = HEADER_SIZE;
            for (ProgramHeader segment: segments) {
                byte_count += SEGMENT_HEADER_SIZE+segment.getFileSize();
            }

            ProgressMonitor monitor = verbose ?
                new ProgressMonitor(byte_count,print_stream) : null;

            byte[] header_bytes = new byte[HEADER_SIZE];
    		ByteBuffer byte_buffer = ByteBuffer.wrap(header_bytes);
    		//buffer.order(ByteOrder.BIG_ENDIAN);
    		byte_buffer.putInt((int)header.getEntryPoint());
    		byte_buffer.putInt(segments.length);

    		ByteArrayInputStream byte_stream = new ByteArrayInputStream(header_bytes);
    		//Send number of headers here
    		transmitter.send(byte_stream,header_bytes.length,monitor);

            for(ProgramHeader segment : segments)
            {
                long segment_filesize = segment.getFileSize();
                long segment_memsize = segment.getMemorySize();
                long segment_file_offset = segment.getFileOffset();
                long segment_offset = segment.getPhysicalAddress();

        		byte[] segment_header_bytes = new byte[SEGMENT_HEADER_SIZE];
            	//Adding the header size and offset as the first 12 bytes of the stream
        		byte_buffer = ByteBuffer.wrap(segment_header_bytes);
        		byte_buffer.putInt((int)segment_filesize);
        		byte_buffer.putInt((int)segment_offset);
        		byte_buffer.putInt((int)segment_memsize);
        		byte_stream = new ByteArrayInputStream(segment_header_bytes);

        		FileInputStream file_stream = new FileInputStream(file);                
        		file_stream.skip(segment_file_offset);

        		SequenceInputStream merged_stream =
        		    new SequenceInputStream(byte_stream, file_stream);
        		transmitter.send(merged_stream,
        		                 segment_header_bytes.length+(int)segment_filesize,
        		                 monitor);

        		file_stream.close();
            }
            if (verbose) {
                print_stream.println();
            }

            while (true)
            {
                int c = in_stream.read();

                // We exit when seeing magic code "\0x"
                // The byte after the magic is the return code
                if (c == '\0') {
                    c = in_stream.read();
                    if (c == 'x') {
                        c = in_stream.read();
                        if (verbose) {
                            print_stream.println();
                            print_stream.println("EXIT "+c);
                        }
                        System.exit(c);
                    } else {
                        print_stream.print('\0');
                        print_stream.print((char)c);
                    }
                } else {
                    print_stream.print((char)c);
                }
            }
        }
        catch (Exception exc)
        {
            print_stream.println(exc);
            error = true;
        }
        finally
        {
			if(port != null)
        	{
				port.closePort();
        	}
        }

        if(error)
        {
            System.exit(-1);
        }
    }

}
