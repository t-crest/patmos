module PatmosUartP {
  provides interface StdControl;
  provides interface UartByte;
  provides interface UartStream;
  uses interface Timer<TMicro> as Timer0;
}
implementation {
  bool stop;

  command error_t StdControl.start() 
  {
    return SUCCESS;
  }

  command error_t StdControl.stop() 
  {
    return FAIL;
  }

  async command error_t UartByte.send( uint8_t byte ) 
  {
    if(!call UartByte.sendAvail())
      return FAIL;

    UART_BUFFER = byte;
    return SUCCESS;
  }

  async command bool UartByte.sendAvail() 
  {
    if(UART_CTRL & (1 << TX_TRANSMIT_READY))
        return TRUE;    
    return FALSE;
  }

  async command error_t UartByte.receive( uint8_t* byte, uint8_t timeout) 
  {
    if(!timeout)
    {
      if(call UartByte.receiveAvail())
      {
        *byte = UART_BUFFER;
        return SUCCESS;
      }
    }
    else
    {
      call Timer0.startOneShot(timeout<<10);
      atomic stop = FALSE;

      while(!call UartByte.receiveAvail() && !stop);
      if(!stop)
      {
        call Timer0.stop();
        *byte = UART_BUFFER;
        return SUCCESS;
      }

    }

    return FAIL;
  }

  async command bool UartByte.receiveAvail() 
  {
    if(UART_CTRL & (1 << RX_DATA_AVAILABLE))
        return TRUE;    
    return FALSE;
  }

  async command error_t UartStream.send( uint8_t* buf, uint16_t len ) 
  {
    uint16_t i;

    for(i = 0; i < len; i++)
    {
      while(!call UartByte.sendAvail());
      call UartByte.send(buf[i]);
    }

    return SUCCESS;
  }

  async command error_t UartStream.enableReceiveInterrupt() 
  {
    return FAIL;
  }

  async command error_t UartStream.disableReceiveInterrupt() 
  {
    return FAIL;
  }

  async command error_t UartStream.receive( uint8_t* buf, uint16_t len) 
  {
    uint16_t i;

    for(i = 0; i < len; i++)
    {
      uint8_t temp;

      while(!call UartByte.receiveAvail());
      temp = UART_BUFFER;
      if(temp != '\n')
        buf[i] = temp;
      else
      {
        buf[i] = '\0';
        break;
      }
    }

    return SUCCESS;
  }

  event void Timer0.fired()
  {
    atomic stop = TRUE;
  }
}
