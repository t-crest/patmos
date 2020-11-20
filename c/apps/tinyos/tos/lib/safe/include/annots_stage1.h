/* Keep TinyOS happy with pre 1.3.0 nesC's */

#ifndef ANNOTS_STAGE1_INCLUDED
#define ANNOTS_STAGE1_INCLUDED

// JDR temporary code: define away two obsolete annotations
#define BOUND(x,y)
#define SINGLE

// JDR temporary code: define away NTS in unsafe mode
#ifndef SAFE_TINYOS
#ifndef NTS
#define NTS
#endif
#endif

#if NESC < 130 

#define __DEPUTY_UNUSED__                      __attribute__((unused))

#define NONNULL                                
#define BND(x,y)                             
#define BND_NOK(x,y)                             
#define COUNT(x)                               
#define COUNT_NOK(x)                               
#define ONE 
#define ONE_NOK
#define DMEMSET(x,y,z)
#define DMEMCPY(x,y,z)
#define TRUSTEDBLOCK
#define NTS

#define TCAST(__type,__expr)                   ((__type)(__expr))                

#ifdef SAFE_TINYOS
#warning Safe TinyOS requires nesC >= 1.3.0
#endif

#ifdef NESC
struct @safe { };
struct @unsafe { };
#endif

#endif // NESC version check

#endif
