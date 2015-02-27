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
// Basic definitions of interfaces to simulate the main memory of Patmos.
//

#ifndef PATMOS_MEMORY_H
#define PATMOS_MEMORY_H

#include "basic-types.h"
#include "command-line.h"

#include <map>
#include <iostream>

namespace patmos
{
  class simulator_t;
  struct stats_options_t;
  
  /// Basic interface to access main memory during simulation.
  class memory_t
  {
  public:
    virtual ~memory_t() {}
    
    /// A simulated access to a read port.
    /// @param s The core performing the access
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size) = 0;

    /// A simulated access to a write port.
    /// @param s The core performing the access
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size) = 0;

    /// A simulated access to a read port to read a fixed size.
    /// @param s The core performing the access
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @return True when the data is available from the read port.
    template<typename T>
    inline bool read_fixed(simulator_t &s, uword_t address, T &value)
    {
      return read(s, address, (byte_t*)&value, sizeof(T));
    }

    /// A simulated access to a write port a fixed size.
    /// @param address The memory address to write to.
    /// @param s The core performing the access
    /// @param value The value to be written to the memory.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    template<typename T>
    bool write_fixed(simulator_t &s, uword_t address, T &value)
    {
      return write(s, address, (byte_t*)&value, sizeof(T));
    }

    /// Read some values from the memory -- DO NOT SIMULATE TIMING, just read.
    /// @param s The core performing the access
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void read_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size) = 0;

    /// Write some values into the memory -- DO NOT SIMULATE TIMING, just write.
    /// @param s The core performing the access
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    virtual void write_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size) = 0;

    
    /// Read some values of a fixed size from the memory -- DO NOT SIMULATE TIMING, just read.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    template<typename T>
    inline void peek_fixed(simulator_t &s, uword_t address, T &value)
    {
      read_peek(s, address, (byte_t*)&value, sizeof(T));
    }

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready() = 0;

    /// Notify the memory that a cycle has passed.
    virtual void tick(simulator_t &s) = 0;

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const = 0;

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(const simulator_t &s, std::ostream &os,
                             const stats_options_t& options) = 0;
    
    /// Reset statistics.
    virtual void reset_stats() = 0;
  };

  /// An ideal memory.
  class ideal_memory_t : public memory_t
  {
  protected:
    /// The size of the memory in bytes.
    unsigned int Memory_size;

    /// Offset up to which the memory has been initialized.
    unsigned int Initialized_offset;

    /// The content of the memory.
    byte_t *Content;
    
    /// Optional vector of flags indicating whether a byte has been initialized.
    byte_t *Init_vector;

    bool Randomize;
    
    mem_check_e Mem_check;
    
    /// Ensure that the content is initialize up to the given address.
    /// @param address The address that should be accessed.
    /// @param size The access size.
    /// @param is_write Access is a write or a read
    /// @param ignore_errors Do not throw any exceptions on access errors
    void check_initialize_content(simulator_t &s, uword_t address, uword_t size, 
                                  bool is_read, bool ignore_errors = false);
    
  public:
    /// Construct a new memory instance.
    /// @param memory_size The size of the memory in bytes.
    ideal_memory_t(unsigned int memory_size, bool randomize, 
                   mem_check_e memchk) 
    : Memory_size(memory_size), Initialized_offset(0), 
      Randomize(randomize), Mem_check(memchk)
    {
      Content = new byte_t[memory_size];
      
      if (memchk != MCK_NONE) {
        Init_vector = new byte_t[memory_size];
      } else {
        Init_vector = NULL;
      }
    }
    
    ~ideal_memory_t() {
      if (Content) delete[] Content;
      if (Init_vector) delete[] Init_vector;
    }

    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    /// Read some values from the memory -- DO NOT SIMULATE TIMING.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void read_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    /// Write some values into the memory -- DO NOT SIMULATE TIMING, just write.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    virtual void write_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready()
    {
      // always ready
      return true;
    }

    /// Notify the memory that a cycle has passed.
    virtual void tick(simulator_t &s)
    {
      // do nothing here
    }

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const
    {
      // nothing to be done here
    }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(const simulator_t &s, std::ostream &os, 
                             const stats_options_t& options)
    {
      // nothing to be done here
    }
    
    virtual void reset_stats() {}

  };

  /// Information about an outstanding memory request.
  struct request_info_t
  {
    /// The address of the request.
    uword_t Address;

    /// The size of the request.
    uword_t Size;

    /// Flag indicating whether the request is a load or store.
    bool Is_load;

    /// If true, do not wait for the request to be retrieved, but delete it.
    bool Is_posted;
    
    /// Number of ticks remaining until the request completes.
    unsigned int Num_ticks_remaining;
  };

  /// A memory with fixed access times to transfer fixed-sized blocks.
  /// Memory accesses are performed in blocks (NUM_BLOCK_BYTES) with a fixed 
  /// access delay (Num_ticks_per_block).
  class fixed_delay_memory_t : public ideal_memory_t
  {
  protected:
    /// Set of pending requests processed by the memory.
    typedef std::vector<request_info_t> requests_t;

    typedef std::map<uword_t, uint64_t> request_size_map_t;
    
    /// Memory access time per block in cycles.
    unsigned int Num_ticks_per_burst;

    /// Block transfer size
    unsigned int Num_bytes_per_burst;
    
    /// Enable posted writes.
    unsigned int Num_posted_writes;
    
    /// Number of initial read delay ticks.
    unsigned int Num_read_delay_ticks;
    
    /// Outstanding requests to the memory.
    requests_t Requests;

  private:
    // -------------  Statistics -------------
    
    /// End address of last request
    uword_t Last_address;
    
    /// Last request was a load?
    bool    Last_is_load;
    
    /// Maximum size of the queue
    unsigned Num_max_queue_size;
    
    /// Number of read requests
    uint64_t Num_reads;
    
    /// Number of write requests
    uint64_t Num_writes;
    
    /// Total number of bytes read
    uint64_t Num_bytes_read;
    
    /// Total number of bytes written
    uint64_t Num_bytes_written;
    
    /// Actual number of bytes transferred for reads
    uint64_t Num_bytes_read_transferred;

    /// Actual number of bytes transferred for writes
    uint64_t Num_bytes_write_transferred;
    
    /// Number of consecutive memory requests
    uint64_t Num_consecutive_requests;
    
    /// Number of cycles the memory interface was busy.
    uint64_t Num_busy_cycles;
    
    /// Number of cycles hidden by posted writes.
    uint64_t Num_posted_write_cycles;
    
    /// Track number of requests per request size.
    request_size_map_t Num_requests_per_size;
    
  protected:  
    virtual uword_t get_aligned_size(uword_t address, uword_t size, 
                                     uword_t &aligned_address);
    
    virtual unsigned int get_transfer_ticks(uword_t aligned_address,
                                            uword_t aligned_size, bool is_load, 
                                            bool is_posted);
    
    /// Let one tick pass for the given request.
    virtual void tick_request(request_info_t &req);
    
    /// Find or create a request given an address, size, and load/store flag.
    /// @param address The address of the request.
    /// @param size The number of bytes request by the access.
    /// @param is_load A flag indicating whether the access is a load or store.
    /// @param is_posted A flag indicating whether the store is posted or not.
    /// @return An existing or newly created request info object.
    /// \see request_info_t
    const request_info_t &find_or_create_request(simulator_t &s, 
                                                 uword_t address, uword_t size,
                                                 bool is_load, 
                                                 bool is_posted = false);
    
  public:
    /// Construct a new memory instance.
    /// @param memory_size The size of the memory in bytes.
    /// @param num_bytes_per_burst Memory block size.
    /// @param num_posted_writes Enable posted writes, sets the max size 
    ///                          of the request queue.
    /// @param num_ticks_per_burst Memory access time per block in cycles.
    /// @param Num_read_delay_ticks Number of ticks until a response to a 
    ///                             request is received
    fixed_delay_memory_t(unsigned int memory_size,
                         unsigned int num_bytes_per_burst,
                         unsigned int num_posted_writes,
                         unsigned int num_ticks_per_burst, 
                         unsigned int num_read_delay_ticks,
                         bool randomize, mem_check_e memchk) :
        ideal_memory_t(memory_size, randomize, memchk),
        Num_ticks_per_burst(num_ticks_per_burst),
        Num_bytes_per_burst(num_bytes_per_burst),
        Num_posted_writes(num_posted_writes),
        Num_read_delay_ticks(num_read_delay_ticks), Last_address(0), 
        Last_is_load(false), Num_max_queue_size(0),
        Num_reads(0), Num_writes(0), Num_bytes_read(0), Num_bytes_written(0),
        Num_bytes_read_transferred(0), Num_bytes_write_transferred(0), 
        Num_consecutive_requests(0), Num_busy_cycles(0), 
        Num_posted_write_cycles(0)
    {
    }

    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready();

    /// Notify the memory that a cycle has passed.
    virtual void tick(simulator_t &s);

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const;

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(const simulator_t &s, std::ostream &os, 
                             const stats_options_t& options);
    
    virtual void reset_stats();

  };

  class variable_burst_memory_t : public fixed_delay_memory_t
  {
  private:
    unsigned int Num_bytes_per_page;
    
  protected:
    virtual unsigned int get_transfer_ticks(uword_t aligned_address,
                                            uword_t aligned_size, bool is_load, 
                                            bool is_posted);
  public:
    variable_burst_memory_t(unsigned int memory_size, 
                        unsigned int num_min_bytes_per_burst,
                        unsigned int num_bytes_per_page,
                        unsigned int num_posted_writes,
                        unsigned int num_min_ticks_per_burst,
                        unsigned int num_read_delay_ticks,
                        bool randomize, mem_check_e memchk)
    : fixed_delay_memory_t(memory_size, num_min_bytes_per_burst, 
                           num_posted_writes, 
                           num_min_ticks_per_burst, num_read_delay_ticks,
                           randomize, memchk),
      Num_bytes_per_page(num_bytes_per_page)
    {}

  };
  
  class tdm_memory_t : public fixed_delay_memory_t 
  {
  private:
    uword_t Round_length;
    
    uword_t Round_start;
    
    uword_t Round_counter;
    
    /// True if we are currently sending a request over the NOC
    bool Is_Transferring;
    
  protected:
    virtual unsigned int get_transfer_ticks(uword_t aligned_address,
                                            uword_t aligned_size, bool is_load, 
                                            bool is_posted);

    virtual void tick_request(request_info_t &req);
    
  public:
    tdm_memory_t(unsigned int memory_size, unsigned int num_bytes_per_burst,
                 unsigned int num_posted_writes,
                 unsigned int num_cores,
                 unsigned int cpu_id,
                 unsigned int num_ticks_per_burst,
                 unsigned int num_read_delay_ticks,
                 unsigned int num_refresh_ticks_per_round, 
                 bool randomize, mem_check_e memchk);
    
    virtual void tick(simulator_t &s);
  };
}

#endif // PATMOS_MEMORY_H

