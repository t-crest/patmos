configuration PatmosUartC {
  provides interface StdControl;
  provides interface UartByte;
  provides interface UartStream;
}

implementation 
{
    components PatmosUartP;
    components new TimerMicroC() as Timer0;

    StdControl = PatmosUartP;
    UartByte = PatmosUartP;
    UartStream = PatmosUartP;
    PatmosUartP.Timer0 -> Timer0;
}
