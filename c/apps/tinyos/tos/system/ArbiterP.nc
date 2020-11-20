/*
 * Copyright (c) 2004, Technische Universitat Berlin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the Technische Universitat Berlin nor the names
 *   of its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
 
/**
 * Please refer to TEP 108 for more information about this component and its
 * intended use.<br><br>
 *
 * This component provides the Resource, ResourceRequested, ArbiterInfo, 
 * and ResourceDefaultOwner interfaces and uses the ResourceConfigure interface as
 * described in TEP 108.  It provides arbitration to a shared resource.
 * A Queue is used to keep track of which users have put
 * in requests for the resource.  Upon the release of the resource by one
 * of these users, the queue is checked and the next user
 * that has a pending request will ge granted control of the resource.  If
 * there are no pending requests, then the user of the ResourceDefaultOwner
 * interface gains access to the resource, and holds onto it until
 * another user makes a request.
 *
 * @param <b>default_owner_id</b> -- The id of the default owner of this 
 *        resource
 * 
 * @author Kevin Klues (klues@tkn.tu-berlin.de)
 * @author Philip Levis
 */
 
generic module ArbiterP(uint8_t default_owner_id) @safe() {
  provides {
    interface Resource[uint8_t id];
    interface ResourceRequested[uint8_t id];
    interface ResourceDefaultOwner;
    interface ArbiterInfo;
  }
  uses {
    interface ResourceConfigure[uint8_t id];
    interface ResourceQueue as Queue;
    interface Leds;
  }
}
implementation {

  enum {RES_CONTROLLED, RES_GRANTING, RES_IMM_GRANTING, RES_BUSY};
  enum {default_owner_id = default_owner_id};
  enum {NO_RES = 0xFF};

  uint8_t state = RES_CONTROLLED;
  norace uint8_t resId = default_owner_id;
  norace uint8_t reqResId;
  
  task void grantedTask();
  
  async command error_t Resource.request[uint8_t id]() {
    signal ResourceRequested.requested[resId]();
    atomic {
      if(state == RES_CONTROLLED) {
        state = RES_GRANTING;
        reqResId = id;
      }
      else if (reqResId == id) {
      	return SUCCESS;
      }
      else return call Queue.enqueue(id);
    }
    signal ResourceDefaultOwner.requested();
    return SUCCESS;
  }

  async command error_t Resource.immediateRequest[uint8_t id]() {
    signal ResourceRequested.immediateRequested[resId]();
    atomic {
      if(state == RES_CONTROLLED) {
        state = RES_IMM_GRANTING;
        reqResId = id;
      }
      else return FAIL;
    }
    signal ResourceDefaultOwner.immediateRequested();
    if(resId == id) {
      call ResourceConfigure.configure[resId]();
      return SUCCESS;
    }
    atomic state = RES_CONTROLLED;
    return FAIL;
  }
  
  async command error_t Resource.release[uint8_t id]() {
    atomic {
      if(state == RES_BUSY && resId == id) {
        if(call Queue.isEmpty() == FALSE) {
          reqResId = call Queue.dequeue();
          resId = NO_RES;
          state = RES_GRANTING;
          post grantedTask();
          call ResourceConfigure.unconfigure[id]();
        }
        else {
          resId = default_owner_id;
          state = RES_CONTROLLED;
          call ResourceConfigure.unconfigure[id]();
          signal ResourceDefaultOwner.granted();
        }
        return SUCCESS;
      }
    }
    return FAIL;
  }

  async command error_t ResourceDefaultOwner.release() {
    atomic {
      if(resId == default_owner_id) {
        if(state == RES_GRANTING) {
          post grantedTask();
          return SUCCESS;
        }
        else if(state == RES_IMM_GRANTING) {
          resId = reqResId;
          state = RES_BUSY;
          return SUCCESS;
        }
      }
    }
    return FAIL;
  }
    
  /**
    Check if the Resource is currently in use
  */    
  async command bool ArbiterInfo.inUse() {
    atomic {
      if (state == RES_CONTROLLED)
        return FALSE;
    }
    return TRUE;
  }

  /**
    Returns the current user of the Resource.
    If there is no current user, the return value
    will be 0xFF
  */      
  async command uint8_t ArbiterInfo.userId() {
    atomic {
      if(state != RES_BUSY)
        return NO_RES;
      return resId;
    }
  }

  /**
   * Returns my user id.
   */      
  async command bool Resource.isOwner[uint8_t id]() {
    atomic {
      if(resId == id && state == RES_BUSY) return TRUE;
      else return FALSE;
    }
  }

  async command bool ResourceDefaultOwner.isOwner() {
    atomic return (state == RES_CONTROLLED
            || (resId == default_owner_id
                && (state == RES_GRANTING || state == RES_IMM_GRANTING)));
  }
  
  task void grantedTask() {
    atomic {
      resId = reqResId;
      state = RES_BUSY;
    }
    call ResourceConfigure.configure[resId]();
    signal Resource.granted[resId]();
  }
  
  //Default event/command handlers for all of the other
    //potential users/providers of the parameterized interfaces 
    //that have not been connected to.  
  default event void Resource.granted[uint8_t id]() {
  }
  default async event void ResourceRequested.requested[uint8_t id]() {
  }
  default async event void ResourceRequested.immediateRequested[uint8_t id]() {
  }
  default async event void ResourceDefaultOwner.granted() {
  }
  default async event void ResourceDefaultOwner.requested() {
    call ResourceDefaultOwner.release();
  }
  default async event void ResourceDefaultOwner.immediateRequested() {
  	call ResourceDefaultOwner.release();
  }
  default async command void ResourceConfigure.configure[uint8_t id]() {
  }
  default async command void ResourceConfigure.unconfigure[uint8_t id]() {
  }
}
