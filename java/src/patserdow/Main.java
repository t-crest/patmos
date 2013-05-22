package patserdow;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.SequenceInputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.concurrent.TimeoutException;

import nl.lxtreme.binutils.coff.SectionHeader;
import nl.lxtreme.binutils.elf.Attribute;
import nl.lxtreme.binutils.elf.Elf;
import nl.lxtreme.binutils.elf.ProgramHeader;
import nl.lxtreme.binutils.elf.Section;
import nl.lxtreme.binutils.elf.Symbol;
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
        
        
        
        String PORT = null;
        String FILENAME = null;
        
        switch(args.length)
        {
            case 2:
                PORT = args[0];
                FILENAME = args[1];
                break;
            default:
                System.err.println("Incorrect number of arguments. Usage: java -jar patserdow COMPORT FILENAME");
        }
        
        SerialPort port = null;
        try 
        {
        	File file = new File(FILENAME);
            Elf elf = new Elf(file);
            Attribute attribute = elf.getAttributes();
            System.out.println("CPU type is:"+attribute.getCPU());
            System.out.println("Instruction width is 32 bits:"+(attribute.getWidth()==32));
            System.out.println("File is of type exe:"+(attribute.getType()==Attribute.ELF_TYPE_EXE));
            System.out.println();
            
            
            port = new SerialPort(PORT);
    		System.out.println("Port opened: " + port.openPort());
            System.out.println("Params set: " + port.setParams(BAUD_RATE, 8, 1, 0));
            Transmitter transmitter = new Transmitter(new UARTInputStream(port), new UARTOutputStream(port));
            //Transmitter transmitter = new Transmitter(System.in, System.out);
            
            ArrayList<Section> sections = new ArrayList<Section>();
            
            for (Section section : elf.getSections(Section.SHT_PROGBITS)) 
            {
            	if(section.getSize() > 0)
                {
            		sections.add(section);
                }
    		}
            
            byte[] headerCount = new byte[4];
    		ByteBuffer buffer = ByteBuffer.wrap(headerCount);
    		//buffer.order(ByteOrder.BIG_ENDIAN);
    		buffer.putInt((int)sections.size());
    		ByteArrayInputStream tempStream = new ByteArrayInputStream(headerCount);
    		//Send number of headers here
    		transmitter.send(tempStream,(int)4);
    		
    		
    		
            for(Section section : sections)
            {
                System.out.println("//HEADER");
                long sectionSize = section.getSize();
                long sectionFileOffset = section.getFileOffset();
                
                
            	FileInputStream fileStream = new FileInputStream(file);
                
            	fileStream.skip(sectionFileOffset);
            	
            	//Adding the header size and offset as the first 8 bytes of the stream
            	byte[] headerArray = new byte[8];
        		buffer = ByteBuffer.wrap(headerArray);
        		buffer.putInt((int)sectionSize);
        		buffer.putInt(0);
        		tempStream = new ByteArrayInputStream(headerArray);
        		SequenceInputStream stream = new SequenceInputStream(tempStream, fileStream);
        		transmitter.send(stream,(int)sectionSize+8);
            	fileStream.close();
            	
            	
                
            }
		}
        finally
        {
			if(port != null)
        	{
				port.closePort();
        	}
        }
    }
    
}