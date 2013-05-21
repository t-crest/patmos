package patserdow;

import java.io.FileInputStream;
import java.io.IOException;
import java.util.concurrent.TimeoutException;
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
        /*Elf elf = new Elf(new File("C:\\Projects\\a.out"));
        
        
        
        Attribute attribute = elf.getAttributes();
        System.out.println(attribute.getCPU());
        System.out.println(attribute.getDebugType());
        System.out.println(attribute.getType());
        System.out.println(attribute.getWidth());
        System.out.println();
        
        for(ProgramHeader header : elf.getProgramHeaders())
        {
            System.out.println("//HEADER");
            System.out.println(header.getTypeName());
            System.out.println(header.getType());
            System.out.println(header.getAlignment());
            System.out.println(header.getFileOffset());
            System.out.println(header.getFileSize());
            System.out.println(header.getMemorySize());
            System.out.println(header.getVirtualAddress());
        }
        elf.loadSymbols();
        for(Symbol symbol : elf.getSymtabSymbols())
        {
            System.out.println("//SYMBOL");
            System.out.println(symbol.getName());
            System.out.println(symbol.getType());
            System.out.println(symbol.getSize());
            System.out.println(symbol.getValue());
            System.out.println(symbol.);
            
        }*/
        
        
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
        FileInputStream file = null;
        SerialPort port = null;
        try 
        {
        	file = new FileInputStream(FILENAME);
        	port = new SerialPort(PORT);
    		System.out.println("Port opened: " + port.openPort());
            System.out.println("Params set: " + port.setParams(BAUD_RATE, 8, 1, 0));
            
            Transmitter transmitter = new Transmitter(new UARTInputStream(port), new UARTOutputStream(port));
            transmitter.send(file);
            file.close();
		}
        finally
        {
        	try 
        	{
	    		if(file != null)
	        	{
	        		file.close();
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
    
}