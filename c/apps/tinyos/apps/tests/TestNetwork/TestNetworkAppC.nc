/**
 * TestNetworkC exercises the basic networking layers, collection and
 * dissemination. The application samples DemoSensorC at a basic rate
 * and sends packets up a collection tree. The rate is configurable
 * through dissemination.
 *
 * See TEP118: Dissemination, TEP 119: Collection, and TEP 123: The
 * Collection Tree Protocol for details.
 * 
 * @author Philip Levis
 * @version $Revision: 1.7 $ $Date: 2009-09-16 00:51:50 $
 */
#include "TestNetwork.h"
#include "Ctp.h"

configuration TestNetworkAppC {}
implementation {
  components TestNetworkC, MainC, LedsC, ActiveMessageC;
  components DisseminationC;
  components new DisseminatorC(uint32_t, SAMPLE_RATE_KEY) as Object32C;
  components CollectionC as Collector;
  components new CollectionSenderC(CL_TEST);
  components new TimerMilliC();
  components new DemoSensorC();
  components new SerialAMSenderC(CL_TEST);
  components SerialActiveMessageC;
#ifndef NO_DEBUG
  components new SerialAMSenderC(AM_COLLECTION_DEBUG) as UARTSender;
  components UARTDebugSenderP as DebugSender;
#endif
  components RandomC;
  components new QueueC(message_t*, 12);
  components new PoolC(message_t, 12);

  TestNetworkC.Boot -> MainC;
  TestNetworkC.RadioControl -> ActiveMessageC;
  TestNetworkC.SerialControl -> SerialActiveMessageC;
  TestNetworkC.RoutingControl -> Collector;
  TestNetworkC.DisseminationControl -> DisseminationC;
  TestNetworkC.Leds -> LedsC;
  TestNetworkC.Timer -> TimerMilliC;
  TestNetworkC.DisseminationPeriod -> Object32C;
  TestNetworkC.Send -> CollectionSenderC;
  TestNetworkC.ReadSensor -> DemoSensorC;
  TestNetworkC.RootControl -> Collector;
  TestNetworkC.Receive -> Collector.Receive[CL_TEST];
  TestNetworkC.UARTSend -> SerialAMSenderC.AMSend;
  TestNetworkC.CollectionPacket -> Collector;
  TestNetworkC.CtpInfo -> Collector;
  TestNetworkC.CtpCongestion -> Collector;
  TestNetworkC.Random -> RandomC;
  TestNetworkC.Pool -> PoolC;
  TestNetworkC.Queue -> QueueC;
  TestNetworkC.RadioPacket -> ActiveMessageC;
  
#ifndef NO_DEBUG
  components new PoolC(message_t, 10) as DebugMessagePool;
  components new QueueC(message_t*, 10) as DebugSendQueue;
  DebugSender.Boot -> MainC;
  DebugSender.UARTSend -> UARTSender;
  DebugSender.MessagePool -> DebugMessagePool;
  DebugSender.SendQueue -> DebugSendQueue;
  Collector.CollectionDebug -> DebugSender;
  TestNetworkC.CollectionDebug -> DebugSender;
#endif
  TestNetworkC.AMPacket -> ActiveMessageC;
}
