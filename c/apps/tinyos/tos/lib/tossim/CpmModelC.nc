/*
 * Copyright (c) 2005 Stanford University. All rights reserved.
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

/**
 *
 * CPM (closest-pattern matching) is a wireless noise simulation model
 * based on statistical extraction from empirical noise data.  This
 * model provides far more precise software simulation environment by
 * exploiting time-correlated noise characteristics. For details,
 * please refer to the paper
 *
 * "Improving Wireless Simulation through Noise Modeling." HyungJune
 * Lee and Philip Levis, IPSN 2007. You can find a copy at
 * http://sing.stanford.edu.
 * 
 * @author Hyungjune Lee, Philip Levis
 * @date   Oct 12 2006
 */ 

#include <sim_gain.h>
#include <sim_noise.h>
#include <randomlib.h>
#include "sim_lqi.c"

module CpmModelC {
	provides interface GainRadioModel as Model;
	provides interface Read<uint16_t> as ReadRssi;
}

implementation {

	message_t* outgoing; // If I'm sending, this is my outgoing packet
	bool requestAck;
	bool receiving = 0;  // Whether or not I think I'm receiving a packet
	bool rssi = FALSE;
	bool transmitting = 0; // Whether or not I think I'm tranmitting a packet
	sim_time_t transmissionEndTime; // to check pending transmission
	struct receive_message;
	typedef struct receive_message receive_message_t;

	struct receive_message {
		int source;
		sim_time_t start;
		sim_time_t end;
		double power;
		double reversePower;
		int8_t strength;
		bool lost;
		bool ack;
		message_t* msg;
		receive_message_t* next;
		uint8_t channel;   // MIKE_LIANG: Channel information for this message
		uint8_t lqi;   // MIKE_LIANG
	};

	receive_message_t* outstandingReceptionHead = NULL;

	receive_message_t* allocate_receive_message();
	void free_receive_message(receive_message_t* msg);
	sim_event_t* allocate_receive_event(sim_time_t t, receive_message_t* m);

	bool shouldReceive(double SNR);
	bool checkReceive(receive_message_t* msg);
	double packetNoise(receive_message_t* msg);
	double checkPrr(receive_message_t* msg);

	double timeInMs()   {
		sim_time_t ftime = sim_time();
		int hours, minutes, seconds;
		sim_time_t secondBillionths;
		int temp_time;
		double ms_time;

		secondBillionths = (ftime % sim_ticks_per_sec());
		if (sim_ticks_per_sec() > (sim_time_t)1000000000) {
			secondBillionths /= (sim_ticks_per_sec() / (sim_time_t)1000000000);
		}
		else {
			secondBillionths *= ((sim_time_t)1000000000 / sim_ticks_per_sec());
		}
		temp_time = (int)(secondBillionths/10000);

		if (temp_time % 10 >= 5) {
			temp_time += (10-(temp_time%10));
		}
		else {
			temp_time -= (temp_time%10);
		}
		ms_time = (float)(temp_time/100.0);

		seconds = (int)(ftime / sim_ticks_per_sec());
		minutes = seconds / 60;
		hours = minutes / 60;
		seconds %= 60;
		minutes %= 60;

		ms_time += (hours*3600+minutes*60+seconds)*1000;

		return ms_time;
	}

	//Generate a CPM noise reading
	double noise_hash_generation()   {
		double CT = timeInMs(); 
		uint32_t quotient = ((sim_time_t)(CT*10))/10;
		uint8_t remain = (uint8_t)(((sim_time_t)(CT*10))%10);
		double noise_val;
		uint16_t node_id = sim_node();

		dbg("CpmModelC", "IN: noise_hash_generation()\n");
		if (5 <= remain && remain < 10) {
			noise_val = (double)sim_noise_generate(node_id, sim_mote_get_radio_channel(node_id), quotient+1);
		}
		else {
			noise_val = (double)sim_noise_generate(node_id, sim_mote_get_radio_channel(node_id), quotient);
		}
		dbg("CpmModelC,Tal", "%s: OUT: noise_hash_generation(): %lf\n", sim_time_string(), noise_val);

		return noise_val;
	}

	double packetSnr(receive_message_t* msg) {
		double signalStr = msg->power;
		double noise = noise_hash_generation();
		return (signalStr - noise);
	}

	double arr_estimate_from_snr(double SNR) {
		double beta1 = 0.9794;
		double beta2 = 2.3851;
		double X = SNR-beta2;
		double PSE = 0.5*erfc(beta1*X/sqrt(2));
		double prr_hat = pow(1-PSE, 23*2);
		dbg("CpmModelC,SNRLoss", "SNR is %lf, ARR is %lf\n", SNR, prr_hat);
		if (prr_hat > 1)
			prr_hat = 1.1;
		else if (prr_hat < 0)
			prr_hat = -0.1;

		return prr_hat;
	}

	int shouldAckReceive(double snr) {
		double prr = arr_estimate_from_snr(snr);
		double coin = RandomUniform();
		if ( (prr >= 0) && (prr <= 1) ) {
			if (coin < prr)
				prr = 1.0;
			else
				prr = 0.0;
		}
		return (int)prr;
	}

	void sim_gain_ack_handle(sim_event_t* evt)  {
		// Four conditions must hold for an ack to be issued:
		// 1) Transmitter is still sending a packet (i.e., not cancelled)
		// 2) The packet requested an acknowledgment
		// 3) The transmitter is on
		// 4) The packet passes the SNR/ARR curve
		if (requestAck && // This 
				outgoing != NULL &&
				sim_mote_is_on(sim_node())) {
			receive_message_t* rcv = (receive_message_t*)evt->data;
			double power = rcv->reversePower;
			double noise = packetNoise(rcv);
			double snr = power - noise;
			if (shouldAckReceive(snr)) {
				signal Model.acked(outgoing);
			}
		}
		free_receive_message((receive_message_t*)evt->data);
	}

	sim_event_t receiveEvent;
	// This clear threshold comes from the CC2420 data sheet
	double clearThreshold = -72.0;
	bool collision = FALSE;
	message_t* incoming = NULL;
	int incomingSource;

	command void Model.setClearValue(double value) {
		clearThreshold = value;
		dbg("CpmModelC", "Setting clear threshold to %f\n", clearThreshold);

	}

	command bool Model.clearChannel() {
		dbg("CpmModelC", "Checking clear channel @ %s: %f <= %f \n", sim_time_string(), (double)packetNoise(NULL), clearThreshold);
		return packetNoise(NULL) < clearThreshold;
	}

	void sim_gain_schedule_ack(int source, sim_time_t t, receive_message_t* r) {
		sim_event_t* ackEvent = (sim_event_t*)malloc(sizeof(sim_event_t));

		ackEvent->mote = source;
		ackEvent->force = 1;
		ackEvent->cancelled = 0;
		ackEvent->time = t;
		ackEvent->handle = sim_gain_ack_handle;
		ackEvent->cleanup = sim_queue_cleanup_event;
		ackEvent->data = r;

		sim_queue_insert(ackEvent);
	}

	double prr_estimate_from_snr(double SNR) {
		// Based on CC2420 measurement by Kannan.
		// The updated function below fixes the problem of non-zero PRR
		// at very low SNR. With this function PRR is 0 for SNR <= 3.
		double beta1 = 0.9794;
		double beta2 = 2.3851;
		double X = SNR-beta2;
		double PSE = 0.5*erfc(beta1*X/sqrt(2));
		double prr_hat = pow(1-PSE, 23*2);
		dbg("CpmModelC,SNR", "SNR is %lf, PRR is %lf\n", SNR, prr_hat);
		if (prr_hat > 1)
			prr_hat = 1.1;
		else if (prr_hat < 0)
			prr_hat = -0.1;

		return prr_hat;
	}

	bool shouldReceive(double SNR) {
		double prr = prr_estimate_from_snr(SNR);
		double coin = RandomUniform();
		if ( (prr >= 0) && (prr <= 1) ) {
			if (coin < prr)
				prr = 1.0;
			else
				prr = 0.0;
		}
		return prr;
	}

	bool checkReceive(receive_message_t* msg) {
		double noise = noise_hash_generation();
		receive_message_t* list = outstandingReceptionHead;
		int count = 0;
		noise = pow(10.0, noise / 10.0);
		while (list != NULL) {
			dbg("CpmModelC", "checkReceive: outstanding from %d\n", list->source);
			count++;
			// MIKE_LIANG: Checks for channel
			if (list->channel != sim_mote_get_radio_channel(sim_node())) {
				list = list->next;
				continue;
			}

			if (list != msg) {
				noise += pow(10.0, list->power / 10.0);
			}
			list = list->next;
		}
		noise = 10.0 * log(noise) / log(10.0);
		dbg("CpmModelC", "checkReceive: outstanding count %d noise %lf at %lf\n", count, noise, (double) sim_time() / sim_ticks_per_sec());
		msg->lqi = sim_lqi_generate(msg->power - noise);
		return shouldReceive(msg->power - noise);
	}

	double packetNoise(receive_message_t* msg) {
		double noise = noise_hash_generation();
		receive_message_t* list = outstandingReceptionHead;
		int count = 0;
		noise = pow(10.0, noise / 10.0);
		while (list != NULL) {
			dbg("CpmModelC", "packetReceive: outstanding from %d\n", list->source);
			count++;
			// MIKE_LIANG: Checks for channel
			if (list->channel != sim_mote_get_radio_channel(sim_node())) {
				list = list->next;
				continue;
			}
			if (list != msg) {
				noise += pow(10.0, list->power / 10.0);
			}
			list = list->next;
		}
		noise = 10.0 * log(noise) / log(10.0);
		dbg("CpmModelC", "packetReceive: outstanding count %d noise %lf at %lf\n", count, noise, (double) sim_time() / sim_ticks_per_sec());
		return noise;
	}

	double checkPrr(receive_message_t* msg) {
		return prr_estimate_from_snr(msg->power / packetNoise(msg));
	}


	/* Handle a packet reception. If the packet is being acked,
		 pass the corresponding receive_message_t* to the ack handler,
		 otherwise free it. */
	void sim_gain_receive_handle(sim_event_t* evt) {
		receive_message_t* mine = (receive_message_t*)evt->data;
		receive_message_t* predecessor = NULL;
		receive_message_t* list = outstandingReceptionHead;

		dbg("CpmModelC", "Handling reception event @ %s.\n", sim_time_string());
		while (list != NULL) {
			if (list->next == mine) {
				predecessor = list;
			}
			list = list->next;
		}
		if (predecessor) {
			predecessor->next = mine->next;
		}
		else if (mine == outstandingReceptionHead) { // must be head
			outstandingReceptionHead = mine->next;
		}
		else {
			dbgerror("CpmModelC", "Incoming packet list structure is corrupted: entry is not the head and no entry points to it.\n");
		}
		dbg("CpmModelC,SNRLoss", "Packet from %i to %i\n", (int)mine->source, (int)sim_node());
		if (!checkReceive(mine)) {
			dbg("CpmModelC,SNRLoss", " - lost packet from %i as SNR was too low.\n", (int)mine->source);
			mine->lost = 1;
		}
		// MIKE_LIANG: Checks for channel
		if (mine->channel != sim_mote_get_radio_channel(sim_node())) {
			free_receive_message(mine);
			receiving = 0;
			return;
		}
		if (!mine->lost) {
			// Copy this receiver's packet signal strength to the metadata region
			// of the packet. Note that this packet is actually shared across all
			// receivers: a higher layer performs the copy.
			tossim_metadata_t* meta = (tossim_metadata_t*)(&mine->msg->metadata);
			meta->strength = mine->strength;
			meta->lqi = mine->lqi;

			dbg("CpmModelC,SNRLoss", "-signaling reception\n");
			signal Model.receive(mine->msg);
			if (mine->ack) {
				dbg("CpmModelC", "yes acknowledgment requested, \n");
			}
			else {
				dbg("CpmModelC", "no acknowledgment requested.\n");
			}
			// If we scheduled an ack, receiving = 0 when it completes
			if (mine->ack && signal Model.shouldAck(mine->msg)) {
				dbg("CpmModelC", " scheduling ack.\n");
				sim_gain_schedule_ack(mine->source, sim_time() + 1, mine);
			}
			else { // Otherwise free the receive_message_t*
				dbg("CpmModelC", " should not ack.\n");
				free_receive_message(mine);
			}
			// We're searching for new packets again
			receiving = 0;
		} // If the packet was lost, then we're searching for new packets again
		else {
			if (RandomUniform() < 0.001) {
				dbg("CpmModelC,SNRLoss", "Packet was technically lost, but TOSSIM introduces an ack false positive rate.\n");
				if (mine->ack && signal Model.shouldAck(mine->msg)) {
					dbg_clear("CpmModelC", " scheduling ack.\n");
					sim_gain_schedule_ack(mine->source, sim_time() + 1, mine);
				}
				else { // Otherwise free the receive_message_t*
					free_receive_message(mine);
				}
			}
			else {
				free_receive_message(mine);
			}
			receiving = 0;
			dbg_clear("CpmModelC,SNRLoss", "  -packet was lost.\n");
		}
	}

	// Create a record that a node is receiving a packet,
	// enqueue a receive event to figure out what happens.
	void enqueue_receive_event(int source, sim_time_t endTime, message_t* msg, bool receive, double power, double reversePower) {
		sim_event_t* evt;
		receive_message_t* list;
		receive_message_t* rcv = allocate_receive_message();
		double noiseStr = packetNoise(rcv);
		rcv->source = source;
		rcv->start = sim_time();
		rcv->end = endTime;
		rcv->power = power;
		rcv->reversePower = reversePower;
		// The strength of a packet is the sum of the signal and noise. In most cases, this means
		// the signal. By sampling this here, it assumes that the packet RSSI is sampled at
		// the beginning of the packet. This is true for the CC2420, but is not true for all
		// radios. But generalizing seems like complexity for minimal gain at this point.
		rcv->strength = (int8_t)(floor(10.0 * log(pow(10.0, power/10.0) + pow(10.0, noiseStr/10.0)) / log(10.0)));
		rcv->msg = msg;
		rcv->lost = 0;
		rcv->ack = receive;
		rcv->channel = sim_mote_get_radio_channel(source);   // Sets to the current radio channel
		// If I'm off, I never receive the packet, but I need to keep track of
		// it in case I turn on and someone else starts sending me a weaker
		// packet. So I don't set receiving to 1, but I keep track of
		// the signal strength.

		if (!sim_mote_is_on(sim_node())) { 
			dbg("CpmModelC", "Lost packet from %i due to %i being off\n", source, sim_node());
			rcv->lost = 1;
		}
		else if (!shouldReceive(power - noiseStr)) {
			dbg("CpmModelC,SNRLoss", "Lost packet from %i to %i due to SNR being too low (%i)\n", source, sim_node(), (int)(power - noiseStr));
			rcv->lost = 1;
		}
		else if (sim_mote_get_radio_channel(sim_node()) != sim_mote_get_radio_channel(source)) {   // MIKE_LIANG
			rcv->lost = 1;
		}
		else if (receiving) {
			dbg("CpmModelC,SNRLoss", "Lost packet from %i due to %i being mid-reception\n", source, sim_node());
			rcv->lost = 1;
		}
		else if (transmitting && (rcv->start < transmissionEndTime) && (transmissionEndTime <= rcv->end)) {
			dbg("CpmModelC,SNRLoss", "Lost packet from %i due to %i being mid-transmission, transmissionEndTime %llu\n", source, sim_node(), transmissionEndTime);
			rcv->lost = 1;
		}
		else {
			receiving = 1;
		}

		list = outstandingReceptionHead;
		while (list != NULL) {
			if (list->channel != sim_mote_get_radio_channel(source)) {   // MIKE_LIANG
				list = list->next;
				continue;
			}
			if (!shouldReceive(list->power - rcv->power)) {
				dbg("Gain,SNRLoss", "Going to lose packet from %i with signal %lf as am receiving a packet from %i with signal %lf\n", list->source, list->power, source, rcv->power);
				list->lost = 1;
			}
			list = list->next;
		}

		rcv->next = outstandingReceptionHead;
		outstandingReceptionHead = rcv;
		evt = allocate_receive_event(endTime, rcv);
		sim_queue_insert(evt);

	}

	void sim_gain_put(int dest, message_t* msg, sim_time_t endTime, bool receive, double power, double reversePower) {
		int prevNode = sim_node();
		dbg("CpmModelC", "Enqueing reception event for %i at %llu with power %lf.\n", dest, endTime, power);
		sim_set_node(dest);
		enqueue_receive_event(prevNode, endTime, msg, receive, power, reversePower);
		sim_set_node(prevNode);
	}

	command void Model.putOnAirTo(int dest, message_t* msg, bool ack, sim_time_t endTime, double power, double reversePower) {
		receive_message_t* list;
		gain_entry_t* neighborEntry = sim_gain_first(sim_node());
		requestAck = ack;
		outgoing = msg;
		transmissionEndTime = endTime;
		dbg("CpmModelC", "Node %i transmitting to %i, finishes at %llu.\n", sim_node(), dest, endTime);

		while (neighborEntry != NULL) {
			int other = neighborEntry->mote;
			sim_gain_put(other, msg, endTime, ack, power + sim_gain_value(sim_node(), other), reversePower + sim_gain_value(other, sim_node()));
			neighborEntry = sim_gain_next(neighborEntry);
		}

		list = outstandingReceptionHead;
		while (list != NULL) {    
			list->lost = 1;
			dbg("CpmModelC,SNRLoss", "Lost packet from %i because %i has outstanding reception, startTime %llu endTime %llu\n", list->source, sim_node(), list->start, list->end);
			list = list->next;
		}
	}


	command void Model.setPendingTransmission() {
		transmitting = TRUE;
		dbg("CpmModelC", "setPendingTransmission: transmitting %i @ %s\n", transmitting, sim_time_string());
	}


	default event void Model.receive(message_t* msg) {}

	sim_event_t* allocate_receive_event(sim_time_t endTime, receive_message_t* msg) {
		sim_event_t* evt = (sim_event_t*)malloc(sizeof(sim_event_t));
		evt->mote = sim_node();
		evt->time = endTime;
		evt->handle = sim_gain_receive_handle;
		evt->cleanup = sim_queue_cleanup_event;
		evt->cancelled = 0;
		evt->force = 1; // Need to keep track of air even when node is off
		evt->data = msg;
		return evt;
	}

	receive_message_t* allocate_receive_message() {
		return (receive_message_t*)malloc(sizeof(receive_message_t));
	}

	void free_receive_message(receive_message_t* msg) {
		free(msg);
	}

	task void read_rssi_task()
	{
		double noise = packetNoise(NULL);
		rssi = FALSE;
		dbg("CpmModelC", "ReadRssi: noise %f\n", noise);
		// The CC2420's RSSI has an offset of 45dBm. Reference: section 23
		// from the CC2420 Datasheet.
		signal ReadRssi.readDone(SUCCESS, noise + 45);
	}

	command error_t ReadRssi.read()
	{
		if (!rssi) {
			rssi = TRUE;
			post read_rssi_task();
			return SUCCESS;
		}
		return FAIL;
	}

	default event void ReadRssi.readDone(error_t error, uint16_t data) { }

}
