#include "hardware.h"

configuration PlatformC { 
  provides interface Init;
}
implementation {
  components PlatformP;
  components HplPatmosGeneralIOC as IO;
  components PatmosMMUC;

  Init = PlatformP;
  PlatformP.Led0 -> IO.Out0;
  PlatformP.Led1 -> IO.Out1;
  PlatformP.Led2 -> IO.Out2;
  PlatformP.Led3 -> IO.Out3;
  PlatformP.Led4 -> IO.Out4;
  PlatformP.Led5 -> IO.Out5;
  PlatformP.Led6 -> IO.Out6;
  PlatformP.Led7 -> IO.Out7;
  PlatformP.Led8 -> IO.Out8;
  PlatformP.MMU -> PatmosMMUC;
}
