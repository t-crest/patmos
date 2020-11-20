
generic module HplPatmosGeneralIOPinP (uint8_t bit) @safe()
{
  provides interface GeneralIO as IO;
}
implementation
{
  inline async command bool IO.get()        { return KEYS & (1 << bit); }
  inline async command void IO.set()        { LED |= (1 << bit); }
  inline async command void IO.clr()        { LED &= ~(1 << bit); }
  inline async command void IO.toggle()     { LED ^= (1 << bit); }
    
  // dummy declarations
  inline async command void IO.makeInput()  {  }
  inline async command bool IO.isInput() {  }
  inline async command void IO.makeOutput() {  }
  inline async command bool IO.isOutput() {  }
}

