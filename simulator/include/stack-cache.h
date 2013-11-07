//
//  This file is part of the Patmos Simulator.
//  The Patmos Simulator is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  The Patmos Simulator is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with the Patmos Simulator. If not, see <http://www.gnu.org/licenses/>.
//
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
    virtual void write_peek(uword_t address, byte_t *value, uword_t size);
    
    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready();
    
  public:
    virtual ~stack_cache_t() {}
    
    /// Prepare for reserveing a given number of bytes, and update the stack 
    /// pointers.
    /// @param size The number of bytes to be reserved.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return the number of bytes to be spilled.
    virtual word_t prepare_reserve(uword_t size, 
                                   uword_t &stack_spill, uword_t &stack_top) = 0;

    /// Prepare for freeing a given number of bytes on the stack, and update
    /// update the stack pointers.
    /// @param size The number of bytes to be freed.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return the number of bytes to be spilled or filled.
    virtual word_t prepare_free(uword_t size, 
                                uword_t &stack_spill, uword_t &stack_top) = 0;

    /// Prepare for ensuring that a given number of bytes are actually 
    /// on the stack, and update the stack pointers.
    /// @param size The number of bytes that have to be available.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return the number of bytes to be filled.
    virtual word_t prepare_ensure(uword_t size, 
                                  uword_t &stack_spill, uword_t &stack_top) = 0;

    /// Prepare for spilling the given number of bytes from the stack, and 
    /// update the stack pointers.
    /// @param size The number of bytes that have to be spilled.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return the number of bytes to be spilled.                                  
    virtual word_t prepare_spill(uword_t size, 
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
    virtual bool reserve(uword_t size, word_t delta,
                         uword_t new_spill, uword_t new_top) = 0;

    /// Free a given number of bytes on the stack.
    /// @param size The number of bytes to be freed.
    /// @param delta The value returned by prepare, i.e., the number of bytes to
    /// be spilled or filled.
    /// @param new_spill The new value of the stack spill pointer.
    /// @param new_top The new value of the stack top pointer.
    /// @return True when the stack space is actually freed in the cache, false
    /// otherwise.
    virtual bool free(uword_t size, word_t delta,
                      uword_t new_spill, uword_t new_top) = 0;

    /// Ensure that a given number of bytes are actually on the stack.
    /// @param size The number of bytes that have to be available.
    /// @param delta The value returned by prepare, i.e., the number of bytes to
    /// be filled.
    /// @param new_spill The new value of the stack spill pointer.
    /// @param new_top The new value of the stack top pointer.
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool ensure(uword_t size, word_t delta,
                        uword_t new_spill, uword_t new_top) = 0;

    /// Spill the given number of bytes from the stack.
    /// @param size The number of bytes that have to be spilled.
    /// @param delta The value returned by prepare, i.e., the number of bytes to
    /// be spilled.
    /// @param new_spill The new value of the stack spill pointer.
    /// @param new_top The new value of the stack top pointer.
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool spill(uword_t size, word_t delta,
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

    
    virtual word_t prepare_reserve(uword_t size, 
                                   uword_t &stack_spill, uword_t &stack_top);
  
    virtual word_t prepare_free(uword_t size, 
                                uword_t &stack_spill, uword_t &stack_top);

    virtual word_t prepare_ensure(uword_t size, 
                                  uword_t &stack_spill, uword_t &stack_top);

    virtual word_t prepare_spill(uword_t size, 
                                 uword_t &stack_spill, uword_t &stack_top);
        
    virtual bool reserve(uword_t size, word_t delta,
                         uword_t new_spill, uword_t new_top);

    virtual bool free(uword_t size, word_t delta,
                      uword_t new_spill, uword_t new_top);

    virtual bool ensure(uword_t size, word_t delta,
                        uword_t new_spill, uword_t new_top);

    virtual bool spill(uword_t size, word_t delta,
                       uword_t new_spill, uword_t new_top);
    
    
    virtual bool read(uword_t address, byte_t *value, uword_t size);

    virtual bool write(uword_t address, byte_t *value, uword_t size);

    virtual void read_peek(uword_t address, byte_t *value, uword_t size);

    virtual void tick() {}

    
    /// Print the internal state of the stack cache to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const;

    virtual void print_stats(const simulator_t &s, std::ostream &os) {}

    virtual void reset_stats() {}

    virtual uword_t size() const;

  };

  /// A stack cache organized in blocks.
  /// The cache is organized in blocks (Num_blocks) each a fixed size in bytes
  /// Num_block_bytes. Spills and fills are performed automatically during the
  /// reserve and ensure instructions.
  class block_stack_cache_t : public ideal_stack_cache_t
  {
  private:
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

    /// Return the number of blocks currently reserved.
    inline unsigned int get_num_reserved_blocks(uword_t spill, uword_t top) const
    {
      return (spill - top) / Num_block_bytes;
    }
  public:
    /// Construct a black-based stack cache.
    /// @param memory The memory to spill/fill.
    /// @param num_blocks Size of the stack cache in blocks.
    block_stack_cache_t(memory_t &memory, unsigned int num_blocks, 
                        unsigned int num_block_bytes);

    virtual ~block_stack_cache_t();

    
    virtual word_t prepare_reserve(uword_t size, 
                                   uword_t &stack_spill, uword_t &stack_top);
  
    virtual word_t prepare_free(uword_t size, 
                                uword_t &stack_spill, uword_t &stack_top);

    virtual word_t prepare_ensure(uword_t size, 
                                  uword_t &stack_spill, uword_t &stack_top);

    virtual word_t prepare_spill(uword_t size, 
                                 uword_t &stack_spill, uword_t &stack_top);
        
    virtual bool reserve(uword_t size, word_t delta,
                         uword_t new_spill, uword_t new_top);

    virtual bool free(uword_t size, word_t delta,
                      uword_t new_spill, uword_t new_top);

    virtual bool ensure(uword_t size, word_t delta,
                        uword_t new_spill, uword_t new_top);

    virtual bool spill(uword_t size, word_t delta,
                       uword_t new_spill, uword_t new_top);


    virtual bool read(uword_t address, byte_t *value, uword_t size);

    virtual bool write(uword_t address, byte_t *value, uword_t size);

    
    virtual void print(std::ostream &os) const;

    virtual void print_stats(const simulator_t &s, std::ostream &os);

    virtual void reset_stats();
    
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

