interface Exception 
{
  async command void enable();
  async command void disable();
  async command void clearPending();
  async event void fired();
}