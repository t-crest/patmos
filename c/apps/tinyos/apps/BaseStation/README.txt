README for BaseStation
Author/Contact: tinyos-help@millennium.berkeley.edu

Description:

BaseStation is an application that acts as a simple Active Message
bridge between the serial and radio links. It replaces the GenericBase
of TinyOS 1.0 and the TOSBase of TinyOS 1.1.

On the serial link, BaseStation sends and receives simple active
messages (not particular radio packets): on the radio link, it sends
radio active messages, whose format depends on the network stack being
used. BaseStation will copy its compiled-in group ID to messages
moving from the serial link to the radio, and will filter out incoming
radio messages that do not contain that group ID.

BaseStation includes queues in both directions, with a guarantee that
once a message enters a queue, it will eventually leave on the other
interface. The queues allow the BaseStation to handle load spikes more
gracefully.

BaseStation acknowledges a message arriving over the serial link only if
that message was successfully enqueued for delivery to the radio link.

The LEDS are programmed to toggle as follows:

RED Toggle         - Message bridged from serial to radio
GREEN Toggle       - Message bridged from radio to serial
YELLOW/BLUE Toggle - Dropped message due to queue overflow 
                     in either direction

When using a CC2420 radio, several default preprocessor configurations
are defined in the Makefile:
  * CC2420_NO_ACKNOWLEDGEMENTS
    - Prevents the base station from falsly acknowledging packets
  * CC2420_NO_ADDRESS_RECOGNITION
    - Allows the base station to sniff packets from any transmitter

Other combinations can be defined to meet your application's needs:
  * CC2420_NO_ADDRESS_RECOGNITION only
    - Sniff all packets, but acknowledge packets only if they
      are sent to the base station's address

  * Removing all preprocessor definitions in the Makefile
    - Only accept packets destined for the base station's address,
      and acknowledge those packets


Tools:

support/sdk/java/net/tinyos/sf/SerialForwarder  

See the TinyOS Tutorial on Mote-PC serial communication and
SerialForwarder on docs.tinyos.net for more details.

Known bugs/limitations:


