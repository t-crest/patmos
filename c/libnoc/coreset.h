/*
   Copyright 2014 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

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

/** \addtogroup coreset
 *  @{
 */

/**
 * \file coreset.h Definitions for sets of cores as used by libnoc.
 * 
 * \author Wolfgang Puffitsch <wpuffitsch@gmail.com>
 *
 */

#ifndef _CORESET_H_
#define _CORESET_H_

/// \brief The maximum number of cores supported by the library. May
/// be changed by defining it before including coreset.h. Should be a
/// power of 2.
#ifndef CORESET_SIZE
#define CORESET_SIZE 32
#endif

#if CORESET_SIZE > 32

typedef unsigned long int __core_mask;
#define __ELEMBITS (8 * sizeof (__core_mask))
#define __ELEMCNT  ((CORESET_SIZE) / __ELEMBITS)

/// \brief A type to describe a set of cores.
typedef struct {
  __core_mask __bits[__ELEMCNT];
} coreset_t;

/// \brief Remove all cores from the set.
/// \param set A set of cores.
static inline void coreset_clearall(coreset_t *set) {
  for (unsigned i = 0; i < __ELEMCNT; i++) {
    set->__bits[i] = 0;
  }
}
/// \brief Add a core to the set.
/// \param core A core number.
/// \param set A set of cores.
static inline void coreset_add(unsigned core, coreset_t *set) {
  set->__bits[core / __ELEMBITS] |= (1 << (core % __ELEMBITS));
}
/// \brief Remove a core from the set.
/// \param core A core number.
/// \param set A set of cores.
static inline void coreset_remove(unsigned core, coreset_t *set) {
  set->__bits[core / __ELEMBITS] &= ~(1 << (core % __ELEMBITS));
}

/// \brief Determins whether the set contains the core.
/// \param core A core number.
/// \param set A set of cores.
/// \returns Non-zero if a core is in the set, zero otherwise.
static inline int coreset_contains(unsigned core, const coreset_t *set) {
  return set->__bits[core / __ELEMBITS] & (1 << (core % __ELEMBITS));
}

/// \brief Determins whether the set is empty.
/// \param set A set of cores.
/// \returns Non-zero if the coreset is empty, zero otherwise.
static inline int coreset_empty(const coreset_t* set) {
  int is_empty = 1;
  for (int i = 0; i < __ELEMCNT; ++i) {
    if(set->__bits[i] != 0) {
      is_empty = 0;
    }
  }
  return is_empty;
}

#elif CORESET_SIZE == 32

typedef unsigned long int __core_mask;
#define __ELEMBITS (8 * sizeof (__core_mask))
#define __ELEMCNT  ((CORESET_SIZE) / __ELEMBITS)

/// \brief A type to describe a set of cores.
typedef struct {
  __core_mask __bits;
} coreset_t;

/// \brief Remove all cores from the set.
/// \param set A set of cores.
static inline void coreset_clearall(coreset_t *set) {
  set->__bits = 0;
}
/// \brief Add a core to the set.
/// \param core A core number.
/// \param set A set of cores.
static inline void coreset_add(unsigned core, coreset_t *set) {
  set->__bits |= (1 << (core % __ELEMBITS));
}
/// \brief Remove a core from the set.
/// \param core A core number.
/// \param set A set of cores.
static inline void coreset_remove(unsigned core, coreset_t *set) {
  set->__bits &= ~(1 << (core % __ELEMBITS));
}

/// \brief Determins whether the set contains the core.
/// \param core A core number.
/// \param set A set of cores.
/// \returns Non-zero if a core is in the set, zero otherwise.
static inline int coreset_contains(unsigned core, const coreset_t *set) {
  return set->__bits & (1 << (core % __ELEMBITS));
}

/// \brief Determins whether the set is empty.
/// \param set A set of cores.
/// \returns Non-zero if the coreset is empty, zero otherwise.
static inline int coreset_empty(const coreset_t* set) {
  int is_empty = 1;
  if(set->__bits != 0) {
    is_empty = 0;
  }
  return is_empty;
}

#endif

#endif /* _CORESET_H_ */
/** @}*/