//$Id: TOSSerial.java,v 1.7 2010-06-29 22:07:41 scipio Exp $

/* Copyright (c) 2000-2003 The Regents of the University of California.  
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the copyright holder nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//@author Cory Sharp <cssharp@eecs.berkeley.edu>
package net.tinyos.comm;

import java.io.*;
import java.util.*;
import java.util.regex.*;

public class TOSSerial extends NativeSerial implements SerialPort {

  /**
   * Inner Class to handle serial event dispatching
   * 
   */
  class EventDispatcher extends Thread {
    private boolean m_run;

    /**
     * Constructor
     * 
     */
    public EventDispatcher() {
      m_run = false;
    }

    /**
     * Start waiting for events
     * 
     */
    public void open() {
      m_run = true;
      if( ! this.isAlive() )
        this.start();
    }

    /**
     * Stop waiting for events
     * Here's the deal: we're running a thread here that is calling
     * a function waitForEvent() in the toscomm driver.  We're now waiting for 
     * two events: DATA_AVAILABLE and OUTPUT_EMPTY.  If you call cancelWait(), 
     * nothing happens until the waitForEvent() returns by getting an event 
     * anyway, so if our node isn't generating bytes on its own, we need to
     * force it to make an event so we can get out of that function to avoid
     * a driver crash.
     * 
     * Previously, it never returned because there were no events.  Now we
     * make an event by adding notifyOn(OUTPUT_EMPTY) and then writing a 
     * standard 0x7E sync byte to the serial port and let it tell us that 
     * an event occured.    
     * 
     * When the waitForEvent() function finally exits, we are then able to 
     * tell it, "Oh yea, while you're at it, cancelWait()".  Finally, the
     * EventDispatcher is in a state where the driver is not sitting around
     * waiting for an event to occur. At that point, we can shut down the
     * NativeSerial by calling super.close() elsewhere. 
     * 
     * As far as I can tell, this is the only way to make this work without
     * modifying the actual toscomm driver.
     * 
     * The only other trick I can see to this is sometimes you can't connect
     * immediately after you disconnect.. I added a wait(500) after a disconnect
     * more toward my application layer to prevent my app from trying to
     * reconnect immediately. My JUnit tests, for example, disconnect and
     * reconnect very rapidly as you would expect. 
     */
    public void close() {
      m_run = false;
      
      while (this.isAlive()) {
        write(0x7E);
        cancelWait();
        try {
          synchronized(this) {
            // Wait for the waitForEvent() done event, if it doesn't work after
            // 100 ms, then we try generating that OUTPUT_EMPTY event again.
            wait(100);
          }
        } catch (InterruptedException e) {
          e.printStackTrace();
        }
      }
    }

    /**
     * Dispatch the event if it really occured
     * 
     * @param event
     */
    private void dispatch_event(int event) {
      if (didEventOccur(event)) {
        SerialPortEvent ev = new SerialPortEvent(TOSSerial.this, event);
        synchronized (m_listeners) {
          Iterator i = m_listeners.iterator();
          while (i.hasNext())
            ((SerialPortListener) i.next()).serialEvent(ev);
        }
      }
    }

    public void run() {
      while (m_run) {
        if (waitForEvent()) {
          dispatch_event(SerialPortEvent.DATA_AVAILABLE);
          dispatch_event(SerialPortEvent.OUTPUT_EMPTY);
        }
      }

      // wake up the closing thread 
      synchronized(this) {
        this.notify();
      }
    }
  }

  /**
   * Inner Serial Input Stream Class
   * 
   */
  class SerialInputStream extends InputStream {
    ByteQueue bq = new ByteQueue(128);

    protected void gather() {
      int navail = TOSSerial.this.available();
      if (navail > 0) {
        byte buffer[] = new byte[navail];
        bq.push_back(buffer, 0, TOSSerial.this.read(buffer, 0, navail));
      }
    }

    public int read() {
      gather();
      return bq.pop_front();
    }

    public int read(byte[] b) {
      gather();
      return bq.pop_front(b);
    }

    public int read(byte[] b, int off, int len) {
      gather();
      return bq.pop_front(b, off, len);
    }

    public int available() {
      gather();
      return bq.available();
    }
  }

  /**
   * Inner Serial Output Stream Class
   * 
   */
  class SerialOutputStream extends OutputStream {
    public void write(int b) {
      TOSSerial.this.write(b);
    }

    public void write(byte[] b) {
      TOSSerial.this.write(b, 0, b.length);
    }

    public void write(byte[] b, int off, int len) {
      int nwritten = 0;
      while (nwritten < len)
        nwritten += TOSSerial.this.write(b, nwritten, len - nwritten);
    }
  }

  private SerialInputStream m_in;

  private SerialOutputStream m_out;

  private Vector m_listeners = new Vector();

  private EventDispatcher m_dispatch;

  static String map_portname(String mapstr, String portname) {
    // mapstr is of the form "from1=to1:from2=to2"

    // If "from", "to", and "portname" all end port numbers, then the ports in
    // "from" and "to" are used as a bias for the port in "portname", appended
    // to the "to" string (without its original terminating digits). If more
    // than one port mapping matches, the one with the smallest non-negative
    // port number wins.

    // For instance, if
    // mapstr="com1=COM1:com10=\\.\COM10"
    // then
    // com1 => COM1
    // com3 => COM3
    // com10 => \\.\COM10
    // com12 => \\.\COM12
    // or if
    // mapstr="com1=/dev/ttyS0:usb1=/dev/ttyS100"
    // then
    // com1 => /dev/ttyS0
    // com3 => /dev/ttyS2
    // usb1 => /dev/ttyS100
    // usb3 => /dev/ttyS102

    String maps[] = mapstr.split(":");
    Pattern pkv = Pattern.compile("(.*?)=(.*?)");
    Pattern pnum = Pattern.compile("(.*\\D)(\\d+)");

    Matcher mport = pnum.matcher(portname);
    int match_distance = -1;
    String str_port_to = null;

    for (int i = 0; i < maps.length; i++) {
      Matcher mkv = pkv.matcher(maps[i]);
      if (mkv.matches()) {
        Matcher mfrom = pnum.matcher(mkv.group(1));
        Matcher mto = pnum.matcher(mkv.group(2));
        if (mfrom.matches() && mto.matches() && mport.matches()
            && mfrom.group(1).equalsIgnoreCase(mport.group(1))) {
          int nfrom = Integer.parseInt(mfrom.group(2));
          int nto = Integer.parseInt(mto.group(2));
          int nport_from = Integer.parseInt(mport.group(2));
          int nport_to = nport_from - nfrom + nto;
          int ndist = nport_from - nfrom;

          if ((ndist >= 0)
              && ((ndist < match_distance) || (match_distance == -1))) {
            match_distance = ndist;
            str_port_to = mto.group(1) + nport_to;
          }
        } else if (mkv.group(1).equalsIgnoreCase(portname)) {
          match_distance = 0;
          str_port_to = mkv.group(2);
        }
      }
    }

    return (str_port_to == null) ? portname : str_port_to;
  }

  /**
   * Real Constructor of TOSSerial
   * 
   * @param portname
   */
  public TOSSerial(String portname) {
    super(map_portname(NativeSerial.getTOSCommMap(), portname));
    m_in = new SerialInputStream();
    m_out = new SerialOutputStream();
    m_dispatch = new EventDispatcher();
    m_dispatch.open();
  }

  /**
   * Open the serial port connection
   */
  public boolean open() {
    if (m_dispatch != null) {
      m_dispatch.open();
    }
    return super.open();
  }

  /**
   * Close the serial port connection
   */
  public void close() {
    if (m_dispatch != null) {
      m_dispatch.close();
    }
    super.close();
  }

  public void addListener(SerialPortListener l) {
    synchronized (m_listeners) {
      if (!m_listeners.contains(l))
        m_listeners.add(l);
    }
  }

  public void removeListener(SerialPortListener l) {
    synchronized (m_listeners) {
      m_listeners.remove(l);
    }
  }

  public InputStream getInputStream() {
    return m_in;
  }

  public OutputStream getOutputStream() {
    return m_out;
  }

  /**
   * Finalize the serial port connection, do not expect to open it again
   */
  public void finalize() {
    // Be careful what you call here. The object may never have been
    // created, so the underlying C++ object may not exist, and there's
    // insufficient guarding to avoid a core dump. If you call other
    // methods than super.close() or super.finalize(), be sure to
    // add an if (swigCptr != 0) guard in NativeSerial.java.
    if (m_dispatch != null) {
      m_dispatch.close();
    }

    /*
     * try { if (m_dispatch != null) { m_dispatch.join(); } } catch
     * (InterruptedException e) { }
     */

    super.close();

    try {
      if (m_in != null) {
        m_in.close();
      }

      if (m_out != null) {
        m_out.close();
      }
    } catch (IOException e) {
    }

    m_dispatch = null;
    m_in = null;
    m_out = null;
    super.finalize();
  }
}
