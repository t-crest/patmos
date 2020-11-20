module PatmosTimerMicro64P
{
  provides interface Counter<TMicro, unsigned long long int>;
  provides interface Alarm<TMicro, unsigned long long int>;
  provides interface Init;

  uses interface Exception;
}

implementation
{
  bool set;
  unsigned long long int next_interrupt;

  command error_t Init.init()
  {
    atomic
    {
      set = FALSE;
      next_interrupt = 0;
    }
    call Exception.clearPending();
    __nesc_enable_interrupt();

    return TRUE;
  }

  async command unsigned long long int Counter.get() 
  {
    uint32_t lo, hi;

    atomic
    {
		asm volatile ("" : : : "memory");
        lo = TIMER1_MICROS_LOW_WORD;
        hi = TIMER1_MICROS_HIGH_WORD;
        asm volatile ("" : : : "memory");
    }
    return lo | ((unsigned long long int)hi << 32);
  }

  async command bool Counter.isOverflowPending() 
  {
    return FALSE;
  }

  async command void Counter.clearOverflow() {}

  async command void Alarm.start(unsigned long long int ndt) 
  {
    call Alarm.startAt(call Counter.get(), ndt);
  }

  async command void Alarm.stop() 
  {
    atomic
    {
      asm volatile ("" : : : "memory");     
        TIMER1_MICROS_LOW_WORD = 0;
        TIMER1_MICROS_HIGH_WORD = 0;
      asm volatile ("" : : : "memory");
      call Exception.disable();
      set = FALSE;
    }
  }

  async command bool Alarm.isRunning() 
  {
    atomic return set;
  }

  async command void Alarm.startAt(unsigned long long int t0, unsigned long long int dt) 
  {
    atomic
    {
  		set = TRUE;
  		next_interrupt = t0 + dt;

      asm volatile ("" : : : "memory");     
        TIMER1_MICROS_LOW_WORD = next_interrupt;
        TIMER1_MICROS_HIGH_WORD = next_interrupt >> 32;
      asm volatile ("" : : : "memory");
      call Exception.clearPending();
      call Exception.enable();
    }
  }

  async command unsigned long long int Alarm.getNow() {
    return call Counter.get();
  }

  async command unsigned long long int Alarm.getAlarm() {
    atomic return next_interrupt;
  }

  async event void Exception.fired()
  {
    signal Alarm.fired();
  }
}