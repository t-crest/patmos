README for TestDisseminationAppC

This application will disseminate 2 constant data objects to all nodes
every 20 seconds. Nodes whose TOS_NODE_ID mod 4 equals 1 will act as
disseminators, and all others will act as receivers. 

Every 20 seconds:
* The disseminator toggles its led0 and led1.
  Sim debugging msg: Timer fired.

* The disseminator sends a new 32-bit value and a new 16-bit value.

* When a receiver receives the correct 32-bit value, it toggles led0.
  Sim debugging msg: Received new correct 32-bit value

* When a receiver receives the correct 16-bit value, it toggles led1.
  Sim debugging msg: Received new correct 16-bit value

Thus, in a successful test, you should see all nodes toggling both
led0 and led1 roughly in unison.
