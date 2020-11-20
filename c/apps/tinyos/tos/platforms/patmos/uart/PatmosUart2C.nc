configuration PatmosUart2C {
  provides interface StdControl;
  provides interface UartByte;
  provides interface UartStream;
}

implementation 
{
    components PatmosUart2P;

    StdControl = PatmosUart2P;
    UartByte = PatmosUart2P;
    UartStream = PatmosUart2P;
}
