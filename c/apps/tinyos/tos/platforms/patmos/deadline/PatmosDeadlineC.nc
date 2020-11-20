configuration PatmosDeadlineC {
  provides interface Deadline;
}

implementation 
{
    components PatmosDeadlineP;

    Deadline = PatmosDeadlineP;
}
