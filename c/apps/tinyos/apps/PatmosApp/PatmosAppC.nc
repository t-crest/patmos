configuration PatmosAppC
{
}

implementation
{
  components MainC, McuSleepC;
  components PatmosAppP;

  components PlatformLedsC;
  components HplPatmosGeneralIOC as IO;
  components PatmosUartC;
  components PatmosDeadlineC;
  components PatmosCpuInfoC;
  components PatmosMMUC;
  
  components new PatmosExceptionP(EXCEPTION_ILLEGAL_OPERATION) as TrapOP;
  components new PatmosExceptionP(4) as TrapTest;
  components new TimerMicroC() as Timer0;
  components new TimerMicroC() as Timer1;

  PatmosAppP.Boot -> MainC;

  PlatformLedsC.Led0 <- PatmosAppP.Led0;
  PlatformLedsC.Led1 <- PatmosAppP.Led1;
  PlatformLedsC.Led2 <- PatmosAppP.Led2;
  PlatformLedsC.Led3 <- PatmosAppP.Led3;
  PlatformLedsC.Led4 <- PatmosAppP.Led4;
  PlatformLedsC.Led5 <- PatmosAppP.Led5;
  PlatformLedsC.Led6 <- PatmosAppP.Led6;
  PlatformLedsC.Led7 <- PatmosAppP.Led7;
  PlatformLedsC.Led8 <- PatmosAppP.Led8;

  IO.In0 <- PatmosAppP.Key0;
  IO.In1 <- PatmosAppP.Key1;
  IO.In2 <- PatmosAppP.Key2;
  IO.In3 <- PatmosAppP.Key3;

  PatmosUartC.UartByte <- PatmosAppP;
  PatmosUartC.UartStream <- PatmosAppP;
  PatmosDeadlineC.Deadline <- PatmosAppP; 
  PatmosCpuInfoC.CpuInfo <- PatmosAppP;
  PatmosMMUC.MemoryManagementUnit <- PatmosAppP;

  TrapOP.Exception <- PatmosAppP.TrapOP;
  TrapTest.Exception <- PatmosAppP.TrapTest;

  PatmosAppP.Timer0 -> Timer0;
  PatmosAppP.Timer1 -> Timer1;
  PatmosAppP.McuSleep -> McuSleepC;
}