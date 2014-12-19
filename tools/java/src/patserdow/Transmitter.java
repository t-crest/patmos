/*
   Copyright 2014 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * Transmit data as packets with CRC
 *
 * Authors: Tórur Biskopstø Strøm (torur.strom@gmail.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

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