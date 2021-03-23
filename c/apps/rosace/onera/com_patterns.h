/* ----------------------------------------------------------------------------
 * SchedMCore - A MultiCore Scheduling Framework
 * Copyright (C) 2009-2011, ONERA, Toulouse, FRANCE - LIFL, Lille, FRANCE
 *
 * This file is part of Prelude
 *
 * Prelude is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation ; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Prelude is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY ; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program ; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *---------------------------------------------------------------------------- */

#ifndef com_patterns_H_
#define com_patterns_H_
// Description of communication protocols using ultimately periodic
// words: prefix.(periodic pattern). Each communication uses a circular
// buffer shared between the writer and the reader.

// The task instance i writes to the communication buffer iff proto[n]
// is true (modulo the size of the periodic word). After each write, the
// index where the writer writes is incremented.
struct write_proto_t {
  int* write_pref;
  int wpref_size;
  int* write_pat;
  int wpat_size;
};

// The task instance n must increment the index at which the task reads
// in the communication buffer iff proto[n] is true (modulo the size of the
// periodic word).
struct read_proto_t {
  int* change_pref;
  int rpref_size;
  int* change_pat;
  int rpat_size;
};

/**
 * Returns 1 if instance n must write in the com buffer.
 */
static inline int must_write(struct write_proto_t wp, int n)
{
  if(n < wp.wpref_size)
    return wp.write_pref[n];
  else
    return wp.write_pat[(n-wp.wpref_size)%wp.wpat_size];
}


/**
 * Returns 1 if instance n must change the cell from which it reads.
 */
static inline int must_change(struct read_proto_t rp, int n)
{
  if(n < rp.rpref_size)
    return rp.change_pref[n];
  else
    return rp.change_pat[(n-rp.rpref_size)%rp.rpat_size];
}

#endif
