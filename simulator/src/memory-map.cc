/*
   Copyright 2012 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the Patmos simulator.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

//
// IO mapping implementation.
//

#include "memory-map.h"
#include "excunit.h"
#include "simulation-core.h"
#include "endian-conversion.h"

#include <vector>
#include <ostream>

using namespace patmos;

bool mapped_device_t::is_word_access(uword_t address, uword_t size, uword_t offset) {
  // TODO optionally check for half/byte access (?)
  return address == Base_address + offset && size == 4;
}

uword_t mapped_device_t::get_word(byte_t *value, uword_t size) {
  uword_t data = *((uword_t*)value);
  return (uword_t)from_big_endian<big_uword_t>(data);
}

void mapped_device_t::set_word(byte_t *value, uword_t size, uword_t data) {
  uword_t big_data = to_big_endian<big_uword_t>(data);
  *((uword_t*)value) = big_data;
}


bool cpuinfo_t::read(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  if (is_word_access(address, size, 0x00)) {
    set_word(value, size, Cpu_id);
  } else if (is_word_access(address, size, 0x04)) {
    set_word(value, size, Cpu_freq);
  } else if (is_word_access(address, size, 0x08)) {
    set_word(value, size, Cpu_cnt);
  } else {
    simulation_exception_t::unmapped(address);
  }
  return true;
}

bool cpuinfo_t::write(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  simulation_exception_t::illegal_access(address);
}

void cpuinfo_t::peek(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  if (is_word_access(address, size, 0x00)) {
    set_word(value, size, Cpu_id);
  } else if (is_word_access(address, size, 0x04)) {
    set_word(value, size, Cpu_freq);
  } else if (is_word_access(address, size, 0x08)) {
    set_word(value, size, Cpu_cnt);
  } else {
    mapped_device_t::peek(s, address, value, size);
  }
}

bool perfcounters_t::read(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  // TODO: return actual performance statistics
  if (is_word_access(address, size, 0x00)) {
    set_word(value, size, 0); // M-Cache hits
  } else if (is_word_access(address, size, 0x04)) {
    set_word(value, size, 0); // M-Cache misses
  } else if (is_word_access(address, size, 0x08)) {
    set_word(value, size, 0); // D-Cache hits
  } else if (is_word_access(address, size, 0x0c)) {
    set_word(value, size, 0); // D-Cache misses
  } else if (is_word_access(address, size, 0x10)) {
    set_word(value, size, 0); // S-Cache spill transactions
  } else if (is_word_access(address, size, 0x14)) {
    set_word(value, size, 0); // S-Cache fill transactions
  } else if (is_word_access(address, size, 0x18)) {
    set_word(value, size, 0); // Write combine buffer hits
  } else if (is_word_access(address, size, 0x1c)) {
    set_word(value, size, 0); // Write combine buffer misses
  } else if (is_word_access(address, size, 0x20)) {
    set_word(value, size, 0); // External memory read transaction
  } else if (is_word_access(address, size, 0x24)) {
    set_word(value, size, 0); // External memory write transaction
  } else {
    simulation_exception_t::unmapped(address);
  }
  return true;
}

bool perfcounters_t::write(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  simulation_exception_t::illegal_access(address);
}

bool mmu_t::read(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  simulation_exception_t::unmapped(address);
}

bool mmu_t::write(simulator_t &s, uword_t address, byte_t *value, uword_t size) {

  if (address >= Base_address && address < Base_address+0x20 &&
      is_word_access(address, size, address & 0x1F)) {

    uword_t index = (address & 0x1F) >> 3;

    if ((address & 0x4) == 0) {
      Segments[index].Base = get_word(value, size);
    } else {
      uword_t val = get_word(value, size);
      Segments[index].Perm = val >> 29;
      Segments[index].Length = val & 0x1fffffff;
    }

  } else {
    simulation_exception_t::unmapped(address);
  }

  return true;
}

uword_t mmu_t::xlate(uword_t address, mmu_op_t op) {
  uword_t index = address >> 29;
  uword_t offset = address & 0x1fffffff;

  if (!ExcUnit->privileged()) {
    if (Segments[index].Length != 0 && offset >= Segments[index].Length) {
      simulation_exception_t::illegal_access(address);
    }
    if (op == MMU_RD && (Segments[index].Perm & 0x4) == 0 ||
        op == MMU_WR && (Segments[index].Perm & 0x2) == 0 ||
        op == MMU_EX && (Segments[index].Perm & 0x1) == 0) {
      simulation_exception_t::illegal_access(address);
    }
  }
  return Segments[index].Base + (address & 0x1fffffff);
}

bool led_t::read(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  if (is_word_access(address, size, 0x00)) {
    set_word(value, size, Curr_state);
    return true;
  }
  else {
    simulation_exception_t::unmapped(address);
  }
}

bool led_t::write(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  if (is_word_access(address, size, 0x00)) {
    uword_t state = get_word(value, size);
     
    if (state == Curr_state) return true;
    
    Out_stream << "--- LEDs: [";
    for ( int i = 0; i < 32; i++ ) {
      if (i > 0) Out_stream << " ";
      if (state & (1 << i)) {
	Out_stream << "X";
      } else {
	Out_stream << "-";
      }
    }
    Out_stream << "] ---\n";
    
    Curr_state = state;
    return true;
  }
  else {
    simulation_exception_t::unmapped(address);
  }
}

// EthMac simulation works only under Linux
#ifdef __linux__
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#else
#warning "EthMac simulation supported only under Linux"
#endif

int ethmac_t::alloc_tap(std::string ip_addr) {
  if (ip_addr == "") {
    return -1;
  }

#ifdef __linux__
  struct ifreq ifr;
  int fd;

  // open tunnel device
  if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
    throw std::runtime_error(std::string("Opening tun device: ") + strerror(errno));
  }

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, "pat0", IFNAMSIZ);

  // Create tap device
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
    throw std::runtime_error(std::string("Creating tap device: ") + strerror(errno));
  }

  // We need to create a socket and work on that to set up the tap device
  int skfd;
  if ((skfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0) {
    throw std::runtime_error(std::string("Opening socket for tap device: ") + strerror(errno));
  }

  // Set IP address of device
  struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;
  addr->sin_family = AF_INET;
  inet_pton(AF_INET, ip_addr.c_str(), &addr->sin_addr);
  if(ioctl(skfd, SIOCSIFADDR, (void *) &ifr) < 0) {
    throw std::runtime_error(std::string("Setting address for tap device: ") + strerror(errno));
  }

  // Get device up and running
  if(ioctl(skfd, SIOCGIFFLAGS, (void *) &ifr) < 0) {
    throw std::runtime_error(std::string("Getting flags for tap device: ") + strerror(errno));
  }
  ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
  if(ioctl(skfd, SIOCSIFFLAGS, (void *) &ifr) < 0) {
    throw std::runtime_error(std::string("Setting tap device up and running: ") + strerror(errno));
  }

  return fd;

#else /* !__linux__ */
  throw std::runtime_error("Providing a network interface for EthMac simulation is supported only under Linux");
#endif /* !__linux__ */
}

bool ethmac_t::read(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  uword_t data = 0;
  if (address >= Base_address && address < Base_address+0xf000 &&
      is_word_access(address, size, address & 0xffff)) {
    data = get_word(&buffer[address & 0xffff], size);
  } else if (is_word_access(address, size, 0xf004)) {
    data = (rx_ready << 2) | (tx_ready << 0);
  } else {
    simulation_exception_t::unmapped(address);
  }
  set_word(value, size, data);
  return true;
}

bool ethmac_t::write(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  uword_t data = get_word(value, size);
  if (address >= Base_address && address < Base_address+0xf000 &&
      is_word_access(address, size, address & 0xffff)) {
    set_word(&buffer[address & 0xffff], size, data);
  } else if (is_word_access(address, size, 0xf000)) {
    // ignore
  } else if (is_word_access(address, size, 0xf004)) {
    if (data & 0x4) { rx_ready = false; }
    if (data & 0x1) { tx_ready = false; }
  } else if (is_word_access(address, size, 0xf400)) {
    tx_length = data >> 16;
    tx = (data & 0x8000) != 0;
  } else if (is_word_access(address, size, 0xf404)) {
    tx_addr = data;
  } else if (is_word_access(address, size, 0xf600)) {
    rx_length = data >> 16;
    rx = (data & 0x8000) != 0;
  } else if (is_word_access(address, size, 0xf604)) {
    rx_addr = data;
  } else if (is_word_access(address, size, 0xf040)) {
    // ignore
  } else if (is_word_access(address, size, 0xf044)) {
    // ignore
  } else {
    simulation_exception_t::unmapped(address);
  }
  return true;
}

void ethmac_t::tick(simulator_t &s) {
  if (fd < 0) {
    return;
  }

#ifdef __linux__
  if (tx && !tx_ready) {
    if (::write(fd, &buffer[tx_addr], tx_length) < 0) {
      std::cerr << "error: Cannot write to tap device" << std::endl;
    }
    tx = false;
    tx_ready = true;
  }

  if (rx && !rx_ready) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    if (::poll(&pfd, 1, 0) > 0) {
      ssize_t len = ::read(fd, &buffer[rx_addr], 0x600);
      if (len > 0) {
        rx = false;
        rx_ready = true;
      } else if (len < 0) {
        std::cerr << "error: Cannot read from tap device" << std::endl;
      }
    }
  }
#endif /* __linux__ */
}

/// Map several devices into the address space of another memory device
mapped_device_t& memory_map_t::find_device(uword_t address)
{
  for (AddressList::iterator it = Device_map.begin(), ie = Device_map.end();
       it != ie; ++it)
  {
    if (address >= it->first && address <= it->second) {
      return *Devices[it - Device_map.begin()];
    }
  }
  simulation_exception_t::unmapped(address);
}

void memory_map_t::add_device(mapped_device_t &device) 
{
  Devices.push_back(&device);
  Device_map.push_back(std::make_pair(device.get_base_address(), 
				      device.get_base_address() + device.get_num_mapped_bytes() - 1));
}

bool memory_map_t::read(simulator_t &s, uword_t address, byte_t *value, uword_t size, bool is_fetch)
{
  if (address >= Base_address && address <= High_address) {
    return find_device(address).read(s, address, value, size);
  } else {
    return Memory.read(s, address, value, size, is_fetch);
  }
}

bool memory_map_t::write(simulator_t &s, uword_t address, byte_t *value, uword_t size)
{
  if (address >= Base_address && address <= High_address) {
    return find_device(address).write(s, address, value, size);
  } else {
    return Memory.write(s, address, value, size);
  }
}

void memory_map_t::read_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size, bool is_fetch)
{
  if (address >= Base_address && address <= High_address) {
    find_device(address).peek(s, address, value, size);
  } else {
    Memory.read_peek(s, address, value, size, is_fetch);
  }
}

void memory_map_t::write_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size)
{
  // TODO should we pass that to the mapped devices?
  assert(address < Base_address || address > High_address);
  Memory.write_peek(s, address, value, size);
}

bool memory_map_t::is_ready()
{
  // TODO should we pass that to the mapped devices?
  return Memory.is_ready();
}

void memory_map_t::tick(simulator_t &s)
{
  for (DeviceList::iterator it = Devices.begin(), ie = Devices.end();
       it != ie; ++it) 
  {
    (*it)->tick(s);
  }
  Memory.tick(s);
}

void memory_map_t::print(const simulator_t &s, std::ostream &os) const
{
  for (DeviceList::const_iterator it = Devices.begin(), ie = Devices.end();
       it != ie; ++it) 
  {
    (*it)->print(s, os);
  }      
  Memory.print(s, os);
}

void memory_map_t::print_stats(const simulator_t &s, std::ostream &os, 
			 const stats_options_t& options)
{
  for (DeviceList::const_iterator it = Devices.begin(), ie = Devices.end();
       it != ie; ++it) 
  {
    (*it)->print_stats(s, os, options);
  }
  Memory.print_stats(s, os, options);
}

void memory_map_t::reset_stats()
{
  for (DeviceList::iterator it = Devices.begin(), ie = Devices.end();
       it != ie; ++it) 
  {
    (*it)->reset_stats();
  }      
  Memory.reset_stats(); 
}
