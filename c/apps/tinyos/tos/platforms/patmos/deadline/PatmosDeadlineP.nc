module PatmosDeadlineP {
  provides interface Deadline;
}

implementation {
  async command void Deadline.start(uint count) 
  {
    DEADLINE = count;
    signal Deadline.done(DEADLINE);
  }
}
