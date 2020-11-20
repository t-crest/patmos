import struct, time, serial

class Serial:
  HDLC_FLAG_BYTE = 0x7e
  HDLC_CTLESC_BYTE = 0x7d
  
  TOS_SERIAL_ACTIVE_MESSAGE_ID = 0
  TOS_SERIAL_CC1000_ID = 1
  TOS_SERIAL_802_15_4_ID = 2
  TOS_SERIAL_UNKNOWN_ID = 255
  
  SERIAL_PROTO_ACK = 67
  SERIAL_PROTO_PACKET_ACK = 68
  SERIAL_PROTO_PACKET_NOACK = 69
  SERIAL_PROTO_PACKET_UNKNOWN = 255
  
  __s = None;       # An instance of serial.Serial object
  __debug = True   # Debug mode
  
  def __init__(self, port, baudrate):
     self.__s = serial.Serial(port, baudrate, rtscts=0)
  
  def __format_packet(self, packet):
      return " ".join(["%02x" % p for p in packet]) + " | " + \
             " ".join(["%d" % p for p in packet])
  
  def crc16(self, base_crc, frame_data):
      crc = base_crc
      for b in frame_data:
          crc = crc ^ (b << 8)
          for i in range(0, 8):
              if crc & 0x8000 == 0x8000:
                  crc = (crc << 1) ^ 0x1021
              else:
                  crc = crc << 1
              crc = crc & 0xffff
      return crc
  
  def __encode(self, val, dim):
      output = []
      for i in range(dim):
          output.append(val & 0xFF)
          val = val >> 8
      return output
  
  def __decode(self, v):
      r = long(0)
      for i in v[::-1]:
          r = (r << 8) + i
      return r
  
  def __get_byte(self):
      r = struct.unpack("B", self.__s.read())[0]
      return r
  
  def __put_bytes(self, data):
      for b in data:
          self.__s.write(struct.pack('B', b))
  
  def __unescape(self, packet):
      r = []
      esc = False
      for b in packet:
          if esc:
              r.append(b ^ 0x20)
              esc = False
          elif b == self.HDLC_CTLESC_BYTE:
              esc = True
          else:
              r.append(b)
      return r
  
  def __escape(self, packet):
      r = []
      for b in packet:
          if b == self.HDLC_FLAG_BYTE or b == self.HDLC_CTLESC_BYTE:
              r.append(self.HDLC_CTLESC_BYTE)
              r.append(b ^ 0x20)
          else:
              r.append(b)
      return r
  
  def read_packet(self):
      d = self.__get_byte()
      ts = time.time()
      while d != self.HDLC_FLAG_BYTE:
          d = self.__get_byte()
          ts = time.time()
      packet = [d]
      d = self.__get_byte()
      if d == self.HDLC_FLAG_BYTE:
          d = self.__get_byte()
          ts = time.time()
      else:
          packet.append(d)
      while d != self.HDLC_FLAG_BYTE:
          d = self.__get_byte()
          packet.append(d)
      un_packet = self.__unescape(packet)
      crc = self.crc16(0, un_packet[1:-3])
      packet_crc = self.__decode(un_packet[-3:-1])
      if crc != packet_crc:
          print "Warning: wrong CRC!"
      if self.__debug == True:
          print "Recv:", self.__format_packet(un_packet)
      return (ts, un_packet)
      
  def write_packet(self, am_group, am_id, data):
      # The first byte after SERIAL_PROTO_PACKET_ACK is a sequence
      # number that will be send back by the mote to ack the receive of
      # the data.
      packet = [self.SERIAL_PROTO_PACKET_ACK, 0, self.TOS_SERIAL_ACTIVE_MESSAGE_ID,
                0xff, 0xff,
                0, 0,
                len(data), am_group, am_id] + data;
      crc = self.crc16(0, packet)
      packet.append(crc & 0xff)
      packet.append((crc >> 8) & 0xff)
      packet = [self.HDLC_FLAG_BYTE] + self.__escape(packet) + [self.HDLC_FLAG_BYTE]
      if self.__debug == True:
          print "Send:", self.__format_packet(packet)
      self.__put_bytes(packet)
      
      # Waiting for ACK
      packet = self.read_packet()
      if len(packet) > 1 and len(packet[1]) > 1:
        return ((packet[1])[1] == self.SERIAL_PROTO_ACK)
      return False
  
  def set_debug(self, debug):
      self.__debug = debug

class GenericPacket:
    """ GenericPacket """

    def __decode(self, v):
        r = long(0)
        for i in v:
            r = (r << 8) + i
        return r
    
    def __encode(self, val, dim):
        output = []
        for i in range(dim):
            output.append(int(val & 0xFF))
            val = val >> 8
        output.reverse()
        return output

    def __init__(self, desc, packet = None):
        self.__dict__['_schema'] = [(t, s) for (n, t, s) in desc]
        self.__dict__['_names'] = [n for (n, t, s) in desc]
        self.__dict__['_values'] = []
        offset = 10
        if type(packet) == type([]):
            for (t, s) in self._schema:
                if t == 'int':
                    self._values.append(self.__decode(packet[offset:offset + s]))
                    offset += s
                elif t == 'blob':
                    if s:
                        self._values.append(packet[offset:offset + s])
                        offset += s
                    else:
                        self._values.append(packet[offset:-3])
        elif type(packet) == type(()):
            for i in packet:
                self._values.append(i)
        else:
            for v in self._schema:
                self._values.append(None)

    def __repr__(self):
        return self._values.__repr__()

    def __str__(self):
        return self._values.__str__()

    # Implement the map behavior
    def __getitem__(self, key):
        return self.__getattr__(key)

    def __setitem__(self, key, value):
        self.__setattr__(key, value)

    def __len__(self):
        return len(self._values)

    def keys(self):
        return self._names

    def values(self):
        return self._names

    # Implement the struct behavior
    def __getattr__(self, name):
        if type(name) == type(0):
            return self._names[name]
        else:
            return self._values[self._names.index(name)]

    def __setattr__(self, name, value):
        if type(name) == type(0):
            self._values[name] = value
        else:
            self._values[self._names.index(name)] = value

    # Custom
    def names(self):
        return self._names

    def sizes(self):
        return self._schema

    def payload(self):
        r = []
        for i in range(len(self._schema)):
            (t, s) = self._schema[i]
            if t == 'int':
                r += self.__encode(self._values[i], s)
            else:
                r += self._values[i]
        return r
