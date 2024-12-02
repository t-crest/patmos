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
 * A program to download applications to Patmos via a serial line
 *
 * Authors: Torur Biskopsto Strom (torur.strom@gmail.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

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
import java.net.DatagramSocket;
import java.net.InetAddress;

import nl.lxtreme.binutils.elf.*;
import jssc.*;

public class Main {

    final private static int BAUD_RATE = 115200;

    private static SerialPort port = null;

    private static class ShutDownHook extends Thread {
        public void run() {
            try {
                if(port != null) {
                    port.closePort();
                }
            } catch (Exception exc) {
                System.err.println(exc);
            }
        }
    }

    private static class InputThread extends Thread {
        private InputStream hostInStream;
        private OutputStream outStream;
        public InputThread(InputStream hostInStream, OutputStream outStream) {
            this.hostInStream = hostInStream;
            this.outStream = outStream;
        }
        public void run() {
            try {
                while (true) {
                    int c = hostInStream.read();
                    if (c == -1) {
                        break;
                    }
                    outStream.write(c);
                }
            } catch (Exception exc) {
                System.err.println(exc);
            }
        }
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        boolean verbose = true;
        boolean compress = true;
        boolean udp = false;
        boolean error = false;

        PrintStream msg_stream = System.err;
        InputStream host_in_stream = System.in;
        OutputStream host_out_stream = System.out;
        InputStream in_stream = null;
        OutputStream out_stream = null;
        OutputStream download_stream = null;

        Runtime.getRuntime().addShutdownHook(new ShutDownHook());

        try {
            verbose = System.getProperty("verbose", "false").equals("true");
            compress = System.getProperty("compress", "true").equals("true");
            udp = System.getProperty("udp", "false").equals("true");
            
            if (compress && udp) {
                throw new IllegalArgumentException("Download via UDP does not support compression");
            }

            File file = null;
            switch(args.length) {
            case 2:
                if (!udp) {
                    port = new SerialPort(args[0]);
                    if (verbose) {
                        msg_stream.println("Port opened: " + port.openPort());
                        msg_stream.println("Params set: " + port.setParams(BAUD_RATE, 8, 1, 0));
                    } else {
                        port.openPort();
                        port.setParams(BAUD_RATE, 8, 1, 0);
                    }
                    in_stream = new UARTInputStream(port);
                    out_stream = new UARTOutputStream(port);
                } else {
                    InetAddress destAddress = InetAddress.getByName(args[0]);
                    int destPort = 8888;

                    DatagramSocket socket = new DatagramSocket(8889);
                    socket.setSoTimeout(10 * 1000);
                    in_stream = new UDPInputStream(socket);
                    out_stream = new UDPOutputStream(socket, destAddress, destPort);
                }
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
                msg_stream.println("Elf version is '1': "+(header.getVersion()==1));
                msg_stream.println("CPU type is: "+header.getMachineType());
                msg_stream.println("Instruction width is 32 bits: "+(header.is32bit()));
                msg_stream.println("Is Big Endian: "+header.isBigEndian());
                msg_stream.println("File is of type exe: "+(header.getType()==ElfHeader.ET_EXEC));
                msg_stream.println("Entry point: "+header.getEntryPoint());
                msg_stream.println();
            }

            if (verbose) {
                msg_stream.println("Download compression enabled: " + compress);
                msg_stream.println();
            }
            if (compress) {
                download_stream = new CompressionOutputStream(out_stream);
            } else {
                download_stream = out_stream;
            }
            Transmitter transmitter = new Transmitter(in_stream,download_stream,udp);

            final int HEADER_SIZE = 8;
            final int SEGMENT_HEADER_SIZE = 12;

            ProgramHeader [] tmp_segments = elf.getProgramHeaders();
            ArrayList<ProgramHeader> segments = new ArrayList<ProgramHeader>();
            for (ProgramHeader segment: tmp_segments) {
                if(segment.getType() == ProgramHeader.PT_LOAD) {
                  segments.add(segment);
                }
            }

            int byte_count = HEADER_SIZE;
            for (ProgramHeader segment: segments) {
                byte_count += SEGMENT_HEADER_SIZE+segment.getFileSize();
            }

            ProgressMonitor monitor = verbose ?
                new ProgressMonitor(byte_count,msg_stream) : null;

            byte[] header_bytes = new byte[HEADER_SIZE];
            ByteBuffer byte_buffer = ByteBuffer.wrap(header_bytes);
            //buffer.order(ByteOrder.BIG_ENDIAN);
            byte_buffer.putInt((int)header.getEntryPoint());
            byte_buffer.putInt(segments.size());

            ByteArrayInputStream byte_stream = new ByteArrayInputStream(header_bytes);
            //Send number of headers here
            transmitter.send(byte_stream,header_bytes.length,monitor);

            for(ProgramHeader segment : segments) {
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
                msg_stream.println();
            }

            // Make sure everything is sent
            transmitter.finish();

            if (verbose) {
                if (download_stream instanceof CompressionOutputStream) {
                    CompressionOutputStream compressionStream =
                        (CompressionOutputStream)download_stream;
                    long textSize = compressionStream.getTextSize();
                    long codeSize = compressionStream.getCodeSize();

                    msg_stream.println("sent " + textSize + " raw bytes "+
                                       "compressed to " + codeSize + " bytes "+
                                       "(" +  ((codeSize * 100) / textSize) + "%)");
                }
            }

            // Write data to target in separate thread
            new InputThread(host_in_stream, out_stream).start();

            // Process data from target
            while (true) {
                int c = in_stream.read();

                // We exit when seeing magic code "\0x"
                // The byte after the magic is the return code
                if (c == '\0') {
                    c = in_stream.read();
                    if (c == 'x') {
                        c = in_stream.read();
                        if (verbose) {
                            msg_stream.println();
                            msg_stream.println("EXIT "+c);
                        }
                        System.exit(c);
                    } else {
                        host_out_stream.write('\0');
                        host_out_stream.write(c);
                        host_out_stream.flush();
                    }
                } else {
                    host_out_stream.write(c);
                    host_out_stream.flush();
                }
            }
        }
        catch (Exception exc) {
            msg_stream.println(exc);
            error = true;
        }

        if(error) {
            System.exit(-1);
        }
    }

}
