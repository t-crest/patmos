configuration PlatformLedsC
{ 
  provides interface GeneralIO as Led0;
  provides interface GeneralIO as Led1;
  provides interface GeneralIO as Led2;
  provides interface GeneralIO as Led3;
  provides interface GeneralIO as Led4;
  provides interface GeneralIO as Led5;
  provides interface GeneralIO as Led6;
  provides interface GeneralIO as Led7;
  provides interface GeneralIO as Led8;
  uses interface Init;
}
implementation
{
  components HplPatmosGeneralIOC as IO;
  components PlatformP;

  Led0 = IO.Out0;
  Led1 = IO.Out1;
  Led2 = IO.Out2;
  Led3 = IO.Out3;
  Led4 = IO.Out4;
  Led5 = IO.Out5;
  Led6 = IO.Out6;
  Led7 = IO.Out7;
  Led8 = IO.Out8;
  Init = PlatformP.LedsInit;
}
