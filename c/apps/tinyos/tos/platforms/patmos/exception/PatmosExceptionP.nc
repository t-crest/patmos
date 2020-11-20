generic module PatmosExceptionP (uint8_t exception) @safe()
{
  provides interface Exception;
}

implementation{

  void __attribute__((naked)) ISR(void) 
  {
  	exc_prologue();
  	signal Exception.fired();
  	exc_epilogue();
  } 

  // enable exeptions and interrupts (0-15: Exceptions, 16-31: Interrupts)
  async command void Exception.enable()
  {
  	EXCEPTION_MASK |= (1 << exception);
  	EXCEPTION_HANDLER_VECTOR(exception) = ISR;
  }
  async command void Exception.disable()
  {
  	EXCEPTION_MASK &= ~(1 << exception);
  }

  async command void Exception.clearPending()
  {
    EXCEPTION_MASK &= ~(1 << exception);
  }
}

