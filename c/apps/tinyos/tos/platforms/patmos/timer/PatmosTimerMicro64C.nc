configuration PatmosTimerMicro64C
{
  provides interface Counter<TMicro, unsigned long long int>;
  provides interface Alarm<TMicro, unsigned long long int>;
  provides interface Init;
}

implementation
{
  components new PatmosExceptionP(EXCEPTION_INTERRUPT_USEC) as TimerInterrupt;
  components PatmosTimerMicro64P;

  Alarm = PatmosTimerMicro64P.Alarm;
  Counter = PatmosTimerMicro64P.Counter;
  Init = PatmosTimerMicro64P.Init;

  TimerInterrupt.Exception <- PatmosTimerMicro64P.Exception; 
}