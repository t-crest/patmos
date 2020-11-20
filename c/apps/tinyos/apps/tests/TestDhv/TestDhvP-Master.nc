
module TestDhvP {
  uses interface Leds;
  uses interface StdControl;

  /*
  uses interface DisseminationUpdate<uint16_t> as DisseminationUpdate1;
  uses interface DisseminationValue<uint16_t> as DisseminationValue1;
  */

  // ... INTERFACES

  uses interface Boot;
  uses interface AMSend as SerialSend;
  uses interface SplitControl as SerialControl;
  uses interface SplitControl as AMControl;
}

implementation {
  typedef nx_struct dhv_test_msg_t {
    nx_am_addr_t id;
    nx_uint8_t count;
    nx_uint8_t isOk;
  } dhv_test_msg_t;

  message_t testMsg;

  uint8_t okBit = 1;
  uint16_t data;
  uint8_t count = 0;
  /*
  uint8_t newCount = N;
  */
  // ... NEWCOUNT

  void bookkeep();

  event void SerialControl.startDone(error_t err) {
    if(err != SUCCESS){
      call SerialControl.start();
      return;
    }
    call AMControl.start();
  }
  
  event void AMControl.startDone(error_t err) {
    if(err != SUCCESS){
      call AMControl.start();
      return;
    }
    
    call StdControl.start();
    if(TOS_NODE_ID == 1) {
      data = 0xBEEF;
      dbg("TestDhvP","Updating data items\n");
      /*
      call DisseminationUpdate1.change(&data);
      */
      // ... CHANGES
    }
  }


  event void SerialControl.stopDone(error_t err) { }
  event void AMControl.stopDone(error_t err) {}

  event void Boot.booted() {
    call SerialControl.start();
    dbg("TestDhvP", "Booted at %s\n", sim_time_string());
  }
  /*
  event void DisseminationValue1.changed() {
    uint16_t val = *(uint16_t*) call DisseminationValue1.get();
    if(val != 0xBEEF) { return; }
    bookkeep();
  }
  */

  // ... EVENTS

  void bookkeep() {
    dhv_test_msg_t* dhvTestMsgPtr;

    if(count < newCount) {
      count++;
    }
    dbg("TestDhvP", "Got an update, %u complete now at %s\n", count, sim_time_string());
    call Leds.led0Toggle();

    dhvTestMsgPtr = (dhv_test_msg_t*) call SerialSend.getPayload(&testMsg, 0);
    dhvTestMsgPtr->id = TOS_NODE_ID;
    dhvTestMsgPtr->count = count;
    dhvTestMsgPtr->isOk = okBit;
    call SerialSend.send(0, &testMsg, sizeof(dhv_test_msg_t));
    

    if(newCount == count) {
      dbg("TestDhvP","Dissemination COMPLETE!\n");
      call Leds.set(7);
    }
    
  }

  event void SerialSend.sendDone(message_t* message, error_t err) {

  }
}
