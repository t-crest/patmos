/*
    Test program for the LL/SC SPM.

    It checks that it doesnt suffer from the ABA problem, unlike CAS.

    Authors: Davide Laezza - Roberts Fanning - Wenhao Li
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "libcorethread/corethread.h"

// Values referred to as A and B
#define A_VAL (0x19)
#define B_VAL (0x84)

// Addresses used for synchronization, since message passing is not available.
#define VICTIM_WRITTEN (0xDEADBEEF)
#define ATTACKER_DONE (0xDEADDEAD)

// Whatever this contant means, it is needed
const int NOC_MASTER = 0;

/*
    Shared scratchpad memory address, whatever the actual type.
    Needs to be a macro otherwise the compiler complains about addresses not
    being constant at compile-time.
*/
#define SHARED_SPM (0xE8000000)
volatile _SPM int* shared_addr = (volatile _SPM int *) (SHARED_SPM + 32);
volatile _SPM int* sync_addr = (volatile _SPM int *) (SHARED_SPM + 64);
volatile _SPM int* result_addr = (volatile _SPM int *) (SHARED_SPM + 1024);

/*
    This function writes A_VAL in the shared address, signals that to the
    attacker and then waits for the value to change to B_VAL and back to
    A_VAL. When this has happened, another value is written. However, this
    should fail due to the other writes, even though the value at the location
    is the same the victim has written.
*/
void victim(void* args) {
    int a = *shared_addr;   // So that A_VAL can be written
    *shared_addr = A_VAL;   // Writng A_VAL to shared variable
    a = *shared_addr;       // Start watching this address

    a = *sync_addr;                // So that token can be written.
    *sync_addr = VICTIM_WRITTEN;   // Signaling to the attacker

    // The attacker is doing its ABA stuff
    while (*sync_addr != ATTACKER_DONE);

    *shared_addr = 0xFF;            // Writing a value to the shared address.

    /*
        At the shared address there should still be A_VAL, meaning that even
        though the value is the same, the location has been written to, and
        therefore the last write failed. So, it the write was successful or
        the value is not A_VAL, then the test fails
    */
    if (*result_addr == 0 || *shared_addr != A_VAL) {
        printf("Test failed\n");
    }
    else {
        printf("test successful\n");
    }

}

/*
    This function waits for the victim to write A_VAL at the shared address,
    then it changes it to B_VAL and finally back to A_VAL. The end of this
    operation is signaled to the victim before exiting.
*/
void attack(void* args) {
    while (*sync_addr != VICTIM_WRITTEN);   // Waiting for the victim to write

    int a = *shared_addr;       // So that B_VAL can be written
    *shared_addr = B_VAL;       // Writing B_VAl to the shared variable.
    a = *shared_addr;           // So that A_VAL can be written
    *shared_addr = A_VAL;       // Writing A_VAl back again to the shared variable.

    a = *sync_addr;                 // So that the token can be written
    *sync_addr = ATTACKER_DONE;     // Signaling to the attacker
}

// The main function. Spawns and attacker therad and then acts as a victim.
int main() {
    int a = *sync_addr;               // So that the token can be written

    /*
        Writing a value other than VICTIM_WRITTEN to be sure that the
        attacker doesn't start too early.
    */
    *sync_addr = VICTIM_WRITTEN - 1;

    corethread_create(1, &attack, NULL);  // Starting the attacker
    victim(NULL);                         // Acting as victim

    return 0;
}
