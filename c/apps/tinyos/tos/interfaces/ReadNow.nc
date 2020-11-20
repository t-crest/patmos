/*
 * Copyright (c) 2005 Arch Rock Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the Arch Rock Corporation nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * ARCHED ROCK OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE
 *
 */

/**
 * The ReadNow interface is intended for split-phase low-latency
 * reading of small values. The type of the value is given as a
 * template argument. Because this interface is split-phase, these
 * values may be backed by hardware. Providers of this interface
 * should also provide a Resource interface, and this interface should
 * only be used after the resource has been acquired. Otherwise, the
 * low-latency requirement may be impossible to meet.
 *
 * <p>
 * This interface has the same calling semantics as the Read interface
 * described in TEP 114, except that it is async.
 * 
 * @param val_t the type of the object that will be returned
 *
 * @author Gilman Tolle <gtolle@archrock.com>
 * @version $Revision: 1.5 $ $Date: 2008-06-16 18:58:51 $
 */

interface ReadNow<val_t> {
  /**
   * Initiates a read of the value.
   * 
   * @return SUCCESS if a readDone() event will eventually come back.
   */
  async command error_t read();

  /**
   * Signals the completion of the read().
   *
   * @param result SUCCESS if the read() was successful
   * @param val the value that has been read
   */
  async event void readDone( error_t result, val_t val );
}
