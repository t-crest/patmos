
configuration PlatformSerialC {
  
  provides interface StdControl;
  provides interface UartStream;
  provides interface UartByte;
  
}

implementation {
  
  components new Msp430Uart1C() as UartC;
  UartStream = UartC;  
  UartByte = UartC;
  
  components TelosSerialP;
  StdControl = TelosSerialP;
  TelosSerialP.Msp430UartConfigure <- UartC.Msp430UartConfigure;
  TelosSerialP.Resource -> UartC.Resource;
  
}
