/*
 A small program that multiply to matrices
 Authors: Evangelia Kasapaki, Wolfgang Puffitsch, Sahar Abbaspour
 Copyright: DTU, BSD License
 */
const int NOC_MASTER = 0;
#include <string.h>
#include <machine/spm.h>
#include <stdio.h>
#include "libnoc/noc.h"
#include "include/patio.h"
#ifdef BOOTROM
extern int _stack_cache_base, _shadow_stack_base;
int main(int argc, char **argv);
void _start(void) __attribute__((naked,used));
void _start(void) {
    // setup stack frame and stack cache.
    asm volatile ("mov $r31 = %0;"// initialize shadow stack pointer"
            "mts $ss  = %1;"// initialize the stack cache's spill pointer"
            "mts $st  = %1;"// initialize the stack cache's top pointer"
            : : "r" (&_shadow_stack_base),
            "r" (&_stack_cache_base));
    // configure network interface
    noc_configure();
    // call main()
    main(0, NULL);
    // freeze
    for(;;);
}
#endif
static void master(void);
static void slave(void);
int first_matrix[3][2] = { {0, 1},
                            {2, 3}, 
                            {4, 5}};
int second_matrix[2][3] = {{ 0, 1, 2}, 
                              {3, 4, 5}};
int result_matrix[3][3] = { {0, 0, 0}, 
                            {0, 0, 0}, 
                            {0, 0, 0}};
struct matrix {
    int first_matrix[3][2];
    int second_matrix[2][3];
    int result_matrix[3][3]; 
    int ready; // flag at end so it is transmitted last
};
volatile _SPM struct matrix *spm_in = (volatile _SPM struct matrix *)NOC_SPM_BASE;
volatile _SPM struct matrix *spm_out = (volatile _SPM struct matrix *)NOC_SPM_BASE+1;
int main() {
    if (get_cpuid() == 0) {
    printf("main\n");
    }
        // clear communication areas
        // cannot use memset() for _SPM pointers!
    for (int i = 0; i < sizeof(struct matrix); i++) {
        ((volatile _SPM char *)spm_in)[i] = 0;
        ((volatile _SPM char *)spm_out)[i] = 0;
    }
            
        
    // dispatch on core ID
    if(get_cpuid() == 0) {
        master();
    } else {
        slave();
    }
    
    return 0;
}
static void master(void) {
    // the message is ready
    spm_out->ready = 1;
// Initialize first matrix
    for (int i = 0; i <= 2; i++)
        for (int j = 0; j <= 1; j++)
            spm_out->first_matrix[i][j] = first_matrix[i][j];
// Initialize second matrix
    for (int i = 0; i <= 1; i++)
        for (int j = 0; j <= 2; j++)
            spm_out->second_matrix[i][j] = second_matrix[i][j]; 
// Initialize third matrix
    for (int i = 0; i <= 2; i++) 
        for (int j = 0; j <= 2; j++)
            spm_out->result_matrix[i][j] = 0;
    
    
    // send message to core 1
    noc_send(get_cpuid() + 1, spm_in, spm_out, sizeof(struct matrix));
    WRITE("SENT\n", 5);
    // wait and poll
    while (!spm_in->ready) {
        /* spin */
    }
    WRITE("RCVD first:\n", 12);
// Print first matrix
    for (int i = 0; i <= 2; i++)
        for (int j = 0; j <= 1; j++)
           printf("%i\n", spm_in->first_matrix[i][j]); 
 WRITE("RCVD second:\n", 13);
// Print second matrix
    for (int i = 0; i <= 1; i++)
        for (int j = 0; j <= 2; j++)
           printf("%i\n", spm_in->second_matrix[i][j]); 
 WRITE("RCVD result:\n", 13);
// Print result matrix
    for (int i = 0; i <= 2; i++)
        for (int j = 0; j <= 2; j++)
            printf("%i\n", spm_in->result_matrix[i][j]); 
    return;
}
// Calculate one part of the matrix and put the results in the result_matrix
void calculate_matrix(volatile _SPM struct matrix * matrix_in, int core) {
	
    switch(core) {
        case 1:
            for (int k = 0; k <= 2; k++)
                for (int j = 0; j <= 1; j++) {
                	spm_out->result_matrix[0][k] += matrix_in->first_matrix[0][j] * matrix_in->second_matrix[j][k];
        
                }
            break;
        case 2:
            for (int k = 0; k <= 2; k++)
                for (int j = 0; j <= 1; j++) {
                	spm_out->result_matrix[1][k] += matrix_in->first_matrix[1][j] * matrix_in->second_matrix[j][k];
                }
            break;
        case 3:
            for (int k = 0; k <= 2; k++)
                for (int j = 0; j <= 1; j++) {
                	spm_out->result_matrix[2][k] += matrix_in->first_matrix[2][j] * matrix_in->second_matrix[j][k];
                }
            break;
    } 

}
static void slave(void) {
    // wait and poll until message arrives
    while (!spm_in->ready) {
        /* spin */
    }
// Copy first matrix
    for (int i = 0; i <= 2; i++)
        for (int j = 0; j <= 1; j++)
           spm_out->first_matrix[i][j] = spm_in->first_matrix[i][j]; 
// Copy second matrix
    for (int i = 0; i <= 1; i++)
        for (int j = 0; j <= 2; j++)
           spm_out->second_matrix[i][j] = spm_in->second_matrix[i][j];
// Copy result matrix
    for (int i = 0; i <= 2; i++)
        for (int j = 0; j <= 2; j++)
            spm_out->result_matrix[i][j] = spm_in->result_matrix[i][j]; 
            
    calculate_matrix(spm_in, get_cpuid());
    spm_out->ready = 1;
    // send to next slave
    int rcv_id = (get_cpuid() == (NOC_CORES - 1)) ? 0 : get_cpuid() + 1;
    noc_send(rcv_id, spm_in, spm_out, sizeof(struct matrix));
    return;
}
