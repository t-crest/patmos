
#ifndef _ATOMIC_H_
#define _ATOMIC_H_

#include <machine/spm.h>


/* 
Reading from this address assures that the next read + write 
happen atomically in the SSPM
*/
#define SCHEDULE_SYNC (0xF00BFFFF)

typedef enum {OPEN, LOCKED} lock_t;

/*
Tries to lock the given lock in the shared SPM.
If locking was successfull 1 is returned, otherwise 0.
Does not block.
*/
int try_lock( volatile _SPM lock_t *l);

/*
Keeps trying to lock the given lock until successfull.
This call busy waits (blocks).
*/
void lock( volatile _SPM lock_t *l);

/*
Releases the given lock in the shared SPM.
*/
void release( volatile _SPM lock_t *l);



#endif
