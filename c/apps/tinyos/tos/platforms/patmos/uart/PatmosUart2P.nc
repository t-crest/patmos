//TODO: add timer for receive Timeout

module PatmosUart2P {
  provides interface StdControl;
  provides interface UartByte;
  provides interface UartStream;
}
implementation {
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

    UART2_BUFFER = byte;
    return SUCCESS;
  }

  async command bool UartByte.sendAvail() 
  {
    if(UART2_CTRL & (1 << TX_TRANSMIT_READY))
        return TRUE;    
    return FALSE;
  }

  async command error_t UartByte.receive( uint8_t* byte, uint8_t timeout) 
  {
    if(!timeout)
    {
      if(call UartByte.receiveAvail())
      {
        *byte = UART2_BUFFER;
        return SUCCESS;
      }
    }
    else
    {
      //TODO implement timeout
    }

    return FAIL;
  }

  async command bool UartByte.receiveAvail() 
  {
    if(UART2_CTRL & (1 << RX_DATA_AVAILABLE))
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

    // read len+1 characters to also read the '\n' which is the termination of the input
    for(i = 0; i < len+1; i++)
    {
      uint8_t temp;

      while(!call UartByte.receiveAvail());
      temp = UART2_BUFFER;
      if(temp != '\n')
        buf[i] = temp;
      else
        break;
    }

    return SUCCESS;
  }
}
