package patserdow;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.LinkedList;
import java.util.zip.CRC32;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

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
    
    private LinkedList<Integer> responseQueue = new LinkedList<Integer>();

    private void ack(int response) throws IOException {
        if (responseQueue.isEmpty()) {
            throw new IOException("Receiver replied unexpectedly (response "+response+")");
        }
        int expected = responseQueue.removeFirst();
        if (response != expected) {
            throw new IOException("Receiver did not reply correctly (expected "+expected+" got "+response+")");
        }
    }

    private void send(byte[] buffer, int offset, int length) throws IOException {
        OUTSTREAM.write(buffer, offset, length);

        // Check responses, if available
        while (INSTREAM.available() > 0) {
            ack(INSTREAM.read());
        }
    }
    
    void send(InputStream stream, int length, ProgressMonitor monitor) throws IOException {
        CRC32 CRC = new CRC32();
        byte[] buffer = new byte[FRAME_SIZE_OFFSET+CRC_SIZE+FRAME_SIZE];
        int remaining = length;
        while(remaining > 0) {
            CRC.reset();
            int size = remaining < FRAME_SIZE ? remaining : FRAME_SIZE;
            int read = stream.read(buffer,FRAME_SIZE_OFFSET+CRC_SIZE,size);
            CRC.update(buffer,FRAME_SIZE_OFFSET+CRC_SIZE,read);

            ByteBuffer byteBuffer = ByteBuffer.wrap(buffer);
            byteBuffer.put((byte)read);
            byteBuffer.putInt((int)CRC.getValue());

            responseQueue.addLast((int)(CRC.getValue() & 0xff));

            send(buffer,0,FRAME_SIZE_OFFSET+CRC_SIZE+read);

            remaining -= read;
            if (monitor != null) {
                monitor.update(read);
            }
        }
    }

    void finish() throws IOException, InterruptedException, ExecutionException, TimeoutException {
        // flush any remaining output
        if (OUTSTREAM instanceof CompressionOutputStream) {
            CompressionOutputStream compressionStream = (CompressionOutputStream)OUTSTREAM;
            compressionStream.finish();
        } else {
            OUTSTREAM.flush();
        }

        // wait for responses with timeout
        Callable<Object> callable = new Callable<Object>() {
            public Object call() throws IOException {
                while (!responseQueue.isEmpty()) {
                    ack(INSTREAM.read());
                }
                return null;
            }
        };
        ExecutorService executorService = Executors.newCachedThreadPool();
        Future<Object> task = executorService.submit(callable);
        try {
            Object result = task.get(10, TimeUnit.SECONDS); 
        } catch (TimeoutException exc) {
            throw new TimeoutException("Receiver did not reply ("+responseQueue.size()+" responses missing)");
        }
    }
}