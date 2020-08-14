#ifndef __DEF_ROSACE_TYPES_H
#define __DEF_ROSACE_TYPES_H

#ifdef USE_FLOAT
#define REAL_TYPE float
#else
#define REAL_TYPE double
#endif

typedef unsigned long long uint64_t;

/* we need forward declaration only in order
 * to avoid redefinition in assemblage_vX generated headers
 * Real "#include "assemblage.h" is only done in assemblage_includes.c
 */
struct aircraft_dynamics_outs_t {
    REAL_TYPE Va;
    REAL_TYPE Vz;
    REAL_TYPE q;
    REAL_TYPE az;
    REAL_TYPE h;
};

typedef enum SAMPLE_RANK {
    SPL_T, SPL_VA,SPL_AZ,SPL_Q,SPL_VZ,SPL_H,
    SPL_DELTA_TH_C, SPL_DELTA_E_C,
    SPL_SIZE
} SampleRank_t;

#endif
