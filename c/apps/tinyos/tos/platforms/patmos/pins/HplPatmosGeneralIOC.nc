configuration HplPatmosGeneralIOC
{
  // provides all the ports as raw ports
  provides {
    interface GeneralIO as Out0;
    interface GeneralIO as Out1;
    interface GeneralIO as Out2;
    interface GeneralIO as Out3;
    interface GeneralIO as Out4;
    interface GeneralIO as Out5;
    interface GeneralIO as Out6;
    interface GeneralIO as Out7;
    interface GeneralIO as Out8;

    interface GeneralIO as In0;
    interface GeneralIO as In1;
    interface GeneralIO as In2;
    interface GeneralIO as In3;
  }
}

implementation
{
  components 
    new HplPatmosGeneralIOPinP(0) as Output0,
    new HplPatmosGeneralIOPinP(1) as Output1,
    new HplPatmosGeneralIOPinP(2) as Output2,
    new HplPatmosGeneralIOPinP(3) as Output3,
    new HplPatmosGeneralIOPinP(4) as Output4,
    new HplPatmosGeneralIOPinP(5) as Output5,
    new HplPatmosGeneralIOPinP(6) as Output6,
    new HplPatmosGeneralIOPinP(7) as Output7,
    new HplPatmosGeneralIOPinP(8) as Output8;

  components 
    new HplPatmosGeneralIOPinP(0) as Input0,
    new HplPatmosGeneralIOPinP(1) as Input1,
    new HplPatmosGeneralIOPinP(2) as Input2,
    new HplPatmosGeneralIOPinP(3) as Input3;

  Out0 = Output0;
  Out1 = Output1;
  Out2 = Output2;
  Out3 = Output3;
  Out4 = Output4;
  Out5 = Output5;
  Out6 = Output6;
  Out7 = Output7;
  Out8 = Output8;

  In0 = Input0;
  In1 = Input1;
  In2 = Input2;
  In3 = Input3;
}
