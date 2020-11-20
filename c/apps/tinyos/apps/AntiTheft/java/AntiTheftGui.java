// $Id: AntiTheftGui.java,v 1.5 2010-06-29 22:07:13 scipio Exp $

/*									tab:4
 * Copyright (c) 2000-2003 The Regents of the University  of California.  
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
 * - Neither the name of the University of California nor the names of
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
 *
 * Copyright (c) 2002-2007 Intel Corporation
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE     
 * file. If you do not find these files, copies can be found by writing to
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300, Berkeley, CA, 
 * 94704.  Attention:  Intel License Inquiry.
 */


/**
 * Description:
 * The GUI for the AntiTheft application.
 *
 * @author Bret Hull
 * @author David Gay
 */

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import net.tinyos.message.*;
import net.tinyos.packet.*;
import net.tinyos.util.*;

public class AntiTheftGui implements MessageListener, Messenger {
    MoteIF mote; 		// For talking to the antitheft root node

    /* Various swing components we need to use after initialisation */
    JFrame frame;		// The whole frame
    JTextArea mssgArea;		// The message area
    JTextField fieldInterval;	// The requested check interval

    /* The checkboxes for the requested settings */
    JCheckBox detDarkCb, detAccelCb, repLedCb, repSirenCb, repServerCb,
	repNeighboursCb;

    public AntiTheftGui() {
	try {
	    guiInit();
	    /* Setup communication with the mote and request a messageReceived
	       callback when an AlertMsg is received */
	    mote = new MoteIF(this);
	    mote.registerListener(new AlertMsg(), this);
	}
	catch(Exception e) {
	    e.printStackTrace();
	    System.exit(2);
	}
    }

    /* Build up the GUI using Swing magic. Nothing very exciting here - the
       BagPanel class makes the code a bit cleaner/easier to read. */
    private void guiInit() throws Exception {
	JPanel mainPanel = new JPanel(new BorderLayout());
	mainPanel.setMinimumSize(new Dimension(500, 250));
	mainPanel.setPreferredSize(new Dimension(500, 300));

	/* The message area */
	JScrollPane mssgPanel = new JScrollPane();
	mssgPanel.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
	mssgPanel.setAutoscrolls(true);
	mssgArea = new JTextArea();
	mssgArea.setFont(new java.awt.Font("Monospaced", Font.PLAIN, 20));
	mainPanel.add(mssgPanel, BorderLayout.CENTER);
	mssgPanel.getViewport().add(mssgArea, null);
	
	/* The button area */
	BagPanel buttonPanel = new BagPanel();
	GridBagConstraints c = buttonPanel.c;

	c.fill = GridBagConstraints.HORIZONTAL;
	c.gridwidth = GridBagConstraints.REMAINDER;

	buttonPanel.makeLabel("Detection", JLabel.CENTER);
	c.gridwidth = GridBagConstraints.RELATIVE;
	detDarkCb = buttonPanel.makeCheckBox("Dark", true);
	c.gridwidth = GridBagConstraints.REMAINDER;
	detAccelCb = buttonPanel.makeCheckBox("Movement", false);
	buttonPanel.makeSeparator(SwingConstants.HORIZONTAL);


	buttonPanel.makeLabel("Theft Reports", JLabel.CENTER);
	c.gridwidth = GridBagConstraints.RELATIVE;
	repLedCb = buttonPanel.makeCheckBox("LED", true);
	c.gridwidth = GridBagConstraints.REMAINDER;
	repSirenCb = buttonPanel.makeCheckBox("Siren", false);
	c.gridwidth = GridBagConstraints.RELATIVE;
	repServerCb = buttonPanel.makeCheckBox("Server", false);
	c.gridwidth = GridBagConstraints.REMAINDER;
	repNeighboursCb = buttonPanel.makeCheckBox("Neighbours", false);
	buttonPanel.makeSeparator(SwingConstants.HORIZONTAL);

	buttonPanel.makeLabel("Interval", JLabel.CENTER);
	fieldInterval = buttonPanel.makeTextField(10, null);
	fieldInterval.setText(Integer.toString(Constants.DEFAULT_CHECK_INTERVAL));

	ActionListener settingsAction = new ActionListener() {
		public void actionPerformed(ActionEvent e) {
		    updateSettings();
		}
	    };
	buttonPanel.makeButton("Update", settingsAction);

	mainPanel.add(buttonPanel, BorderLayout.EAST);

	/* The frame part */
	frame = new JFrame("AntiTheft");
	frame.setSize(mainPanel.getPreferredSize());
	frame.getContentPane().add(mainPanel);
	frame.setVisible(true);
	frame.addWindowListener(new WindowAdapter() {
		public void windowClosing(WindowEvent e) { System.exit(0); }
	    });
    }

    /* Add a message to the message area, auto-scroll to end */
    public synchronized void message(String s) {
	mssgArea.append(s + "\n");
	mssgArea.setCaretPosition(mssgArea.getDocument().getLength());
    }

    /* Popup an error message */
    void error(String msg) {
	JOptionPane.showMessageDialog(frame, msg, "Error",
				      JOptionPane.ERROR_MESSAGE);
    }

    /* User pressed the "Update" button. Read the GUI fields and
       send a SettingsMsg with the requested values. When the
       requested settings are bad, we silently update them to sane
       values. */
    public void updateSettings() { 
	SettingsMsg smsg = new SettingsMsg();
	short alert = 0;
	short detect = 0;
	int checkInterval = Constants.DEFAULT_CHECK_INTERVAL;

	/* Extract current interval value, fixing bad values */
	String intervalS = fieldInterval.getText().trim();
	try {
	    int newInterval = Integer.parseInt(intervalS);
	    if (newInterval < 10) throw new NumberFormatException();
	    checkInterval = newInterval;
	}
	catch (NumberFormatException e) { 
	    /* Reset field when value is bad */
	    fieldInterval.setText("" + checkInterval);
	}

	/* Extract alert settings */
	if (repLedCb.isSelected())
	    alert |= Constants.ALERT_LEDS;
	if (repSirenCb.isSelected())
	    alert |= Constants.ALERT_SOUND;
	if (repNeighboursCb.isSelected())
	    alert |= Constants.ALERT_RADIO;
	if (repServerCb.isSelected())
	    alert |= Constants.ALERT_ROOT;
	if (alert == 0) {
	    /* If nothing select, force-select LEDs */
	    alert = Constants.ALERT_LEDS;
	    repLedCb.setSelected(true);
	}

	/* Extract detection settings */
	if (detDarkCb.isSelected())
	    detect |= Constants.DETECT_DARK;
	if (detAccelCb.isSelected())
	    detect |= Constants.DETECT_ACCEL;
	if (detect == 0) {
	    /* If no detection selected, force-select dark */
	    detect = Constants.DETECT_DARK;
	    detDarkCb.setSelected(true);
	}

	/* Build and send settings message */
	smsg.set_alert(alert);
	smsg.set_detect(detect);
	smsg.set_checkInterval(checkInterval);
	try {
	    mote.send(MoteIF.TOS_BCAST_ADDR, smsg);
	}
	catch (IOException e) {
	    error("Cannot send message to mote");
	}
    }

    /* Message received from mote network. Update message area if it's
       a theft message. */
    public void messageReceived(int dest_addr, Message msg) {
	if (msg instanceof AlertMsg) {
	    AlertMsg alertMsg = (AlertMsg)msg;
	    message("Theft of " + alertMsg.get_stolenId());
	}
    }

    /* Just start the app... */
    public static void main(String[] args) {
	AntiTheftGui me = new AntiTheftGui();
    }
}
