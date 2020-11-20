#include "Timer.h"

configuration HilTimerMicroC
{
  provides interface Init;
  provides interface Timer<TMicro> as TimerMicro[uint8_t id];
  provides interface LocalTime<TMicro>;
}
implementation
{
  components new AlarmToTimerC(TMicro);
  components new VirtualizeTimerC(TMicro,uniqueCount(UQ_TIMER_MICRO));
  components new CounterToLocalTimeC(TMicro);
  components PatmosTimerMicro64C;

  Init = PatmosTimerMicro64C;

  TimerMicro = VirtualizeTimerC;
  VirtualizeTimerC.TimerFrom -> AlarmToTimerC;

  LocalTime = CounterToLocalTimeC;

  AlarmToTimerC.Alarm -> PatmosTimerMicro64C.Alarm;
  CounterToLocalTimeC.Counter -> PatmosTimerMicro64C.Counter;
}