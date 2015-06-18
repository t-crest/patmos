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
// Basic definitions of interfaces to simulate the stack cache of Patmos.
//

#ifndef PATMOS_STACK_CACHE_H
#define PATMOS_STACK_CACHE_H

#include "memory.h"

#include <vector>
#include <ostream>

namespace patmos
{
  class simulator_t;
  
  /// Base class for all stack cache implementations.
  class stack_cache_t : public memory_t
  {
  private:
    /// Write some values into the memory -- DO NOT SIMULATE TIMING, just write.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    virtual void write_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size);

  public:    
    virtual ~stack_cache_t() {}

    virtual bool read_burst(simulator_t &s, uword_t address, byte_t *value, 
                            uword_t size, uword_t &transferred,
                            bool low_priority = false);

    virtual bool is_serving_request(uword_t address);
    
    /// Prepare for reserveing a given number of bytes, and update the stack 
    /// pointers.
    /// @param size The number of bytes to be reserved.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return the number of bytes to be spilled.
    virtual word_t prepare_reserve(simulator_t &s, uword_t size, 
                                   uword_t &stack_spill, uword_t &stack_top) = 0;

    /// Prepare for freeing a given number of bytes on the stack, and update
    /// update the stack pointers.
    /// @param size The number of bytes to be freed.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return the number of bytes to be spilled or filled.
    virtual word_t prepare_free(simulator_t &s, uword_t size, 
                                uword_t &stack_spill, uword_t &stack_top) = 0;

    /// Prepare for ensuring that a given number of bytes are actually 
    /// on the stack, and update the stack pointers.
    /// @param size The number of bytes that have to be available.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return the number of bytes to be filled.
    virtual word_t prepare_ensure(simulator_t &s, uword_t size, 
                                  uword_t &stack_spill, uword_t &stack_top) = 0;

    /// Prepare for spilling the given number of bytes from the stack, and 
    /// update the stack pointers.
    /// @param size The number of bytes that have to be spilled.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return the number of bytes to be spilled.                                  
    virtual word_t prepare_spill(simulator_t &s, uword_t size, 
                                 uword_t &stack_spill, uword_t &stack_top) = 0;

    
    /// Reserve a given number of bytes, potentially spilling stack data to some
    /// memory.
    /// @param size The number of bytes to be reserved.
    /// @param delta The value returned by prepare, i.e., the number of bytes to
    /// be spilled.
    /// @param new_spill The new value of the stack spill pointer.
    /// @param new_top The new value of the stack top pointer.
    /// @return True when the stack space is actually reserved on the cache,
    /// false otherwise.
    virtual bool reserve(simulator_t &s, uword_t size, word_t delta,
                         uword_t new_spill, uword_t new_top) = 0;

    /// Free a given number of bytes on the stack.
    /// @param size The number of bytes to be freed.
    /// @param delta The value returned by prepare, i.e., the number of bytes to
    /// be spilled or filled.
    /// @param new_spill The new value of the stack spill pointer.
    /// @param new_top The new value of the stack top pointer.
    /// @return True when the stack space is actually freed in the cache, false
    /// otherwise.
    virtual bool free(simulator_t &s, uword_t size, word_t delta,
                      uword_t new_spill, uword_t new_top) = 0;

    /// Ensure that a given number of bytes are actually on the stack.
    /// @param size The number of bytes that have to be available.
    /// @param delta The value returned by prepare, i.e., the number of bytes to
    /// be filled.
    /// @param new_spill The new value of the stack spill pointer.
    /// @param new_top The new value of the stack top pointer.
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool ensure(simulator_t &s, uword_t size, word_t delta,
                        uword_t new_spill, uword_t new_top) = 0;

    /// Spill the given number of bytes from the stack.
    /// @param size The number of bytes that have to be spilled.
    /// @param delta The value returned by prepare, i.e., the number of bytes to
    /// be spilled.
    /// @param new_spill The new value of the stack spill pointer.
    /// @param new_top The new value of the stack top pointer.
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool spill(simulator_t &s, uword_t size, word_t delta,
                       uword_t new_spill, uword_t new_top) = 0;

    /// Get the current size of the stack cache in bytes.
    virtual uword_t size() const = 0;
  };

  /// An ideal stack cache with "infinite" space.
  class ideal_stack_cache_t : public stack_cache_t
  {
  protected:
    /// The content of the cache.
    std::vector<byte_t> Content;

    /// The memory to spill/fill.
    memory_t &Memory;

  public:
    
    ideal_stack_cache_t(memory_t &memory) : Memory(memory) {}

    virtual word_t prepare_reserve(simulator_t &s, uword_t size, 
                                   uword_t &stack_spill, uword_t &stack_top);
  
    virtual word_t prepare_free(simulator_t &s, uword_t size, 
                                uword_t &stack_spill, uword_t &stack_top);

    virtual word_t prepare_ensure(simulator_t &s, uword_t size, 
                                  uword_t &stack_spill, uword_t &stack_top);

    virtual word_t prepare_spill(simulator_t &s, uword_t size, 
                                 uword_t &stack_spill, uword_t &stack_top);
        
    virtual bool reserve(simulator_t &s, uword_t size, word_t delta,
                         uword_t new_spill, uword_t new_top);

    virtual bool free(simulator_t &s, uword_t size, word_t delta,
                      uword_t new_spill, uword_t new_top);

    virtual bool ensure(simulator_t &s, uword_t size, word_t delta,
                        uword_t new_spill, uword_t new_top);

    virtual bool spill(simulator_t &s, uword_t size, word_t delta,
                       uword_t new_spill, uword_t new_top);
    
    
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    virtual void read_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    virtual void tick(simulator_t &s) {}

    virtual bool is_ready();

    /// Print the internal state of the stack cache to an output stream.
    /// @param os The output stream to print to.
    virtual void print(const simulator_t &s, std::ostream &os) const;

    virtual void print_stats(const simulator_t &s, std::ostream &os, 
                             const stats_options_t& options) {}

    virtual void reset_stats() {}

    virtual uword_t size() const;

  };

  /// A stack cache that sends all request directly through another memory
  /// or cache.
  class proxy_stack_cache_t : public ideal_stack_cache_t
  {
  private:
    /// Remember the current value of the stack top pointer.
    /// Note that this can be different from $st when $st is set explicitly
    /// via MTS until the next STC instruction.
    uword_t stack_top;
    
  public:
    proxy_stack_cache_t(memory_t& memory) 
    : ideal_stack_cache_t(memory), stack_top(0) 
    {}
    
    virtual bool reserve(simulator_t &s, uword_t size, word_t delta,
                         uword_t new_spill, uword_t new_top) 
    { stack_top = new_top; return true; }

    virtual bool free(simulator_t &s, uword_t size, word_t delta,
                      uword_t new_spill, uword_t new_top)
    { stack_top = new_top; return true; }

    virtual bool ensure(simulator_t &s, uword_t size, word_t delta,
                        uword_t new_spill, uword_t new_top)
    { stack_top = new_top; return true; }

    virtual bool spill(simulator_t &s, uword_t size, word_t delta,
                       uword_t new_spill, uword_t new_top)
    { stack_top = new_top; return true; }
    
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    virtual void read_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    virtual bool is_ready();
    
    virtual bool is_serving_request(uword_t address);
  };
  
  /// A stack cache organized in blocks.
  /// The cache is organized in blocks (Num_blocks) each a fixed size in bytes
  /// Num_block_bytes. Spills and fills are performed automatically during the
  /// reserve and ensure instructions.
  class block_stack_cache_t : public ideal_stack_cache_t
  {
  protected:
    /// Possible transfers to/from the stack cache.
    enum phase_e
    {
      /// No transfer ongoing.
      IDLE,
      /// Data is transferred from the stack cache to the memory.
      SPILL,
      /// Data is transferred from the memory to the stack cache.
      FILL
    };

    /// Size of the stack cache in blocks.
    unsigned int Num_blocks;

    /// Size of blocks in bytes.
    unsigned int Num_block_bytes;
    
    /// Store currently ongoing transfer.
    phase_e Phase;

    /// The memory to spill/fill.
    memory_t &Memory;

    /// Temporary buffer used during spill/fill.
    byte_t *Buffer;

    // *************************************************************************
    // statistics

    /// Total number of blocks reserved.
    unsigned int Num_blocks_reserved;

    /// Maximal number of blocks reserved at once.
    unsigned int Max_blocks_reserved;

    /// Total number of blocks transferred to main (spill) memory.
    unsigned int Num_blocks_spilled;

    /// Maximal number of blocks transferred to main at once (spill) memory.
    unsigned int Max_blocks_spilled;

    /// Total number of blocks transferred from main (fill) memory.
    unsigned int Num_blocks_filled;

    /// Maximal number of blocks transferred from main at once (fill) memory.
    unsigned int Max_blocks_filled;

    /// Number of executed free instructions resulting in an entirely empty
    /// stack cache.
    unsigned int Num_free_empty;

    /// Number of read accesses to the stack cache.
    unsigned int Num_read_accesses;

    /// Number of bytes read from the stack cache.
    unsigned int Num_bytes_read;

    /// Number of write accesses to the stack cache.
    unsigned int Num_write_accesses;

    /// Number of bytes written to the stack cache.
    unsigned int Num_bytes_written;

    /// Number of stall cycles caused by method cache misses.
    unsigned int Num_stall_cycles;

    /// Return the number of blocks currently reserved.
    inline unsigned int get_num_reserved_blocks(uword_t spill, uword_t top) const
    {
      return (spill - top) / Num_block_bytes;
    }
  public:
    /// Construct a block-based stack cache.
    /// @param memory The memory to spill/fill.
    /// @param num_blocks Size of the stack cache in blocks.
    block_stack_cache_t(memory_t &memory, unsigned int num_blocks, 
                        unsigned int num_block_bytes);

    virtual ~block_stack_cache_t();

    
    virtual word_t prepare_reserve(simulator_t &s, uword_t size, 
                                   uword_t &stack_spill, uword_t &stack_top);
  
    virtual word_t prepare_free(simulator_t &s, uword_t size, 
                                uword_t &stack_spill, uword_t &stack_top);

    virtual word_t prepare_ensure(simulator_t &s, uword_t size, 
                                  uword_t &stack_spill, uword_t &stack_top);

    virtual word_t prepare_spill(simulator_t &s, uword_t size, 
                                 uword_t &stack_spill, uword_t &stack_top);
        
    virtual bool reserve(simulator_t &s, uword_t size, word_t delta,
                         uword_t new_spill, uword_t new_top);

    virtual bool free(simulator_t &s, uword_t size, word_t delta,
                      uword_t new_spill, uword_t new_top);

    virtual bool ensure(simulator_t &s, uword_t size, word_t delta,
                        uword_t new_spill, uword_t new_top);

    virtual bool spill(simulator_t &s, uword_t size, word_t delta,
                       uword_t new_spill, uword_t new_top);

    virtual bool is_ready();

    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size);


    virtual void print(const simulator_t &s, std::ostream &os) const;

    virtual void print_stats(const simulator_t &s, std::ostream &os, 
                             const stats_options_t& options);

    virtual void reset_stats();
    
  };

  /// A stack cache generating only aligned memory transfers, given a 
  /// pre-defined block size, while preserving the impression that the stack 
  /// cache operates on blocks of 4 bytes (words) for the instruction set 
  /// architecture.
  /// 
  /// The main idea is to reserve one block of the stack cache as a sort of 
  /// alignment buffer, reducing the effective size of the maximal reservable 
  /// space (which has to be respected by the compiler). This allows the stack 
  /// cache to spill/fill entire blocks that are properly aligned.
  /// 
  /// In addition, free instructions need some special care. Whenever the free
  /// empties the stack cache a single block needs to be filled in order to 
  /// ensure proper alignment of the memory top pointer and matching content of 
  /// the stack cache.
  class block_aligned_stack_cache_t : public block_stack_cache_t
  {
    private:
      /// Number of bytes transferred as a block during filling/spilling.
      uword_t Num_transfer_block_bytes;

      /// Total number of words additionally transferred to main (spill) memory.
      unsigned int Num_words_spilled;

      /// Maximal number of blocks additionally transferred to main at once 
      /// (spill) memory.
      unsigned int Max_words_spilled;

      /// Total number of words additionally transferred from main (fill) 
      /// memory.
      unsigned int Num_words_filled;

      /// Maximal number of blocks additionally transferred from main at once 
      /// (fill) memory.
      unsigned int Max_words_filled;

      /// Total number of words additionally transferred from main (free) 
      /// memory.
      unsigned int Num_words_free_filled;

      /// Maximal number of blocks additionally transferred from main at once 
      /// (fill) memory.
      unsigned int Max_words_free_filled;
    public:
      /// Construct an aligned block-based stack cache.
      /// @param memory The memory to spill/fill.
      /// @param num_blocks Size of the stack cache in blocks.
      block_aligned_stack_cache_t(memory_t &memory, unsigned int num_blocks, 
                                  unsigned int num_block_bytes);

      /// Override the original prepare reserve function and align the stack 
      /// spill pointer / transfer size.
      virtual word_t prepare_reserve(simulator_t &s, uword_t size, 
                                     uword_t &stack_spill, uword_t &stack_top);

      // Override the original prepare ensure function and align the stack spill
      // pointer / transfer size.
      virtual word_t prepare_ensure(simulator_t &s, uword_t size, 
                                    uword_t &stack_spill, uword_t &stack_top);

      // Override the original prepare free function and align the stack spill
      // pointer and potentially trigger a one-block fill.
      virtual word_t prepare_free(simulator_t &s, uword_t size, 
                                  uword_t &stack_spill, uword_t &stack_top);

      // Behave as a normal free, but if needed execute a one-block fill.
      virtual bool free(simulator_t &s, uword_t size, word_t delta,
                        uword_t new_spill, uword_t new_top);

      virtual void print_stats(const simulator_t &s, std::ostream &os, 
                               const stats_options_t& options);

      void reset_stats();
  };

  class block_lazy_stack_cache_t : public block_stack_cache_t
  {
    private: 
      /// Pointer relative to stack top stack tracking data that has been 
      /// modified.
      uword_t Lazy_pointer;

      /// Updated next value of the Lazy_pointer.
      /// \see Lazy_pointer
      uword_t Next_Lazy_pointer;
      
      /// Number of blocks that should be evicted but not spilled by the next 
      /// reserve
      uword_t Num_blocks_to_evict;
      
      /// Statistic counter, measuring the number of blocks that were not 
      /// spilled due to lazy spilling.
      unsigned int Num_blocks_not_spilled;
      
      /// Statistic counter, measuring the maximum number of blocks that were 
      /// not spilled due to lazy spilling.
      unsigned int Max_blocks_not_spilled;      
    public:

      /// Construct a lazy block-based stack cache.
      /// @param memory The memory to spill/fill.
      /// @param num_blocks Size of the stack cache in blocks.
      block_lazy_stack_cache_t(memory_t &memory, unsigned int num_blocks, 
                        unsigned int num_block_bytes);

      virtual word_t prepare_reserve(simulator_t &s, uword_t size, uword_t &stack_spill, 
                                     uword_t &stack_top);
      
      virtual word_t prepare_free(simulator_t &s, uword_t size, uword_t &stack_spill, 
                                  uword_t &stack_top);

      /// Free a given number of bytes on the stack.
      /// @param size The number of bytes to be freed.
      /// @param delta The value returned by prepare, i.e., the number of bytes 
      /// to be spilled or filled.
      /// @param new_spill The new value of the stack spill pointer.
      /// @param new_top The new value of the stack top pointer.
      /// @return True when the stack space is actually freed in the cache, 
      /// false otherwise.
      virtual bool free(simulator_t &s, uword_t size, word_t delta,
                        uword_t new_spill, uword_t new_top);

      /// Reserve a given number of bytes, potentially spilling stack data to some
      /// memory.
      /// @param size The number of bytes to be reserved.
      /// @param delta The value returned by prepare, i.e., the number of bytes to
      /// be spilled.
      /// @param new_spill The new value of the stack spill pointer.
      /// @param new_top The new value of the stack top pointer.
      /// @return True when the stack space is actually reserved on the cache,
      /// false otherwise.
      virtual bool reserve(simulator_t &s, uword_t size, word_t delta,
                           uword_t new_spill, uword_t new_top);

      virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size);

      virtual void print(const simulator_t &s, std::ostream &os) const;

      virtual void print_stats(const simulator_t &s, std::ostream &os, 
                               const stats_options_t& options);

      void reset_stats();
  };

  /// Operator to print the state of a stack cache to a stream
  /// @param os The output stream to print to.
  /// @param phase The state of the stack cache.
  template<typename T>
  inline std::ostream &operator <<(std::ostream &os,
                                   typename T::phase_e phase)
  {
    switch (phase)
    {
      case T::IDLE:
        os << "idle";
        break;
      case T::SPILL:
        os << "spill";
        break;
      case T::FILL:
        os << "fill";
        break;
    }

    return os;
  }
}

#endif // PATMOS_STACK_CACHE_H

