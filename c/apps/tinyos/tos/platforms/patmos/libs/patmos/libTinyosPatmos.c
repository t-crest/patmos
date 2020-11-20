#include "../libTinyosPatmos.h"

void trap(unsigned n) {
  asm volatile("trap %0" : : "i" (n));
}

void exc_prologue()
{
	asm volatile("sub  $r31 = $r31, 12;"                   
               /* Save the stack cache state */          
               "swc  [$r31 + 0] = $r1;"                  
               "swc  [$r31 + 1] = $r2;"                  
               "mfs  $r1 = $ss;"                         
               "mfs  $r2 = $st;"                         
               "sub  $r1 = $r1, $r2;"                    
               "swc  [$r31 + 2] = $r1;"                  
               /* Save general-purpose registers */      
               "sres 48;"                                
               "sws  [3] = $r3;"                         
               "sws  [4] = $r4;"                         
               "sws  [5] = $r5;"                         
               "sws  [6] = $r6;"                         
               "sws  [7] = $r7;"                         
               "sws  [8] = $r8;"                         
               "sws  [9] = $r9;"                         
               "sws  [10] = $r10;"                       
               "sws  [11] = $r11;"                       
               "sws  [12] = $r12;"                       
               "sws  [13] = $r13;"                       
               "sws  [14] = $r14;"                       
               "sws  [15] = $r15;"                       
               "sws  [16] = $r16;"                       
               "sws  [17] = $r17;"                       
               "sws  [18] = $r18;"                       
               "sws  [19] = $r19;"                       
               "sws  [20] = $r20;"                       
               "sws  [21] = $r21;"                       
               "sws  [22] = $r22;"                       
               "sws  [23] = $r23;"                       
               "sws  [24] = $r24;"                       
               "sws  [25] = $r25;"                       
               "sws  [26] = $r26;"                       
               "sws  [27] = $r27;"                       
               "sws  [28] = $r28;"                       
               "sws  [29] = $r29;"                       
               "sws  [30] = $r30;"                       
               /* Save special registers */              
               "mfs  $r1 = $s0;"                         
               "mfs  $r2 = $s1;"                         
               "mfs  $r3 = $s2;"                         
               "mfs  $r4 = $s3;"                         
               "mfs  $r5 = $s4;"                         
               "mfs  $r8 = $s7;"                         
               "mfs  $r9 = $s8;"                         
               "mfs  $r10 = $s9;"                        
               "mfs  $r11 = $s10;"                       
               "mfs  $r12 = $s11;"                       
               "mfs  $r13 = $s12;"                       
               "mfs  $r14 = $s13;"                       
               "mfs  $r15 = $s14;"                       
               "mfs  $r16 = $s15;"                       
               "sws  [32] = $r1;"                        
               "sws  [33] = $r2;"                        
               "sws  [34] = $r3;"                        
               "sws  [35] = $r4;"                        
               "sws  [36] = $r5;"                        
               "sws  [39] = $r8;"                        
               "sws  [40] = $r9;"                        
               "sws  [41] = $r10;"                       
               "sws  [42] = $r11;"                       
               "sws  [43] = $r12;"                       
               "sws  [44] = $r13;"                       
               "sws  [45] = $r14;"                       
               "sws  [46] = $r15;"                       
               "sws  [47] = $r16;" : :                   
               /* Clobber everything */                  
               : "$r1", "$r2", "$r3",                    
                 "$r4", "$r5", "$r6", "$r7",             
                 "$r8", "$r9", "$r10", "$r11",           
                 "$r12", "$r13", "$r14", "$r15",         
                 "$r16", "$r17", "$r18", "$r19",         
                 "$r20", "$r21", "$r22", "$r23",         
                 "$r24", "$r25", "$r26", "$r27",         
                 "$r28", "$r29", "$r30", "$r31",         
                 "$s0", "$s1", "$s2", "$s3",             
                 "$s4", "$s5", "$s6", "$s7",             
                 "$s8", "$s9", "$s10", "$s11",           
                 "$s12", "$s13", "$s14", "$s15");
}

void exc_epilogue()
{
 asm volatile(/* Restore special registers */           
               "lws  $r1 = [32];"                        
               "lws  $r2 = [33];"                        
               "lws  $r3 = [34];"                        
               "lws  $r4 = [35];"                        
               "lws  $r5 = [36];"                        
               "lws  $r8 = [39];"                        
               "lws  $r9 = [40];"                        
               "lws  $r10 = [41];"                       
               "lws  $r11 = [42];"                       
               "lws  $r12 = [43];"                       
               "lws  $r13 = [44];"                       
               "lws  $r14 = [45];"                       
               "lws  $r15 = [46];"                       
               "lws  $r16 = [47];"                       
               "mts  $s0 = $r1;"                         
               "mts  $s1 = $r2;"                         
               "mts  $s2 = $r3;"                         
               "mts  $s3 = $r4;"                         
               "mts  $s4 = $r5;"                         
               "mts  $s7 = $r8;"                         
               "mts  $s8 = $r9;"                         
               "mts  $s9 = $r10;"                        
               "mts  $s10 = $r11;"                       
               "mts  $s11 = $r12;"                       
               "mts  $s12 = $r13;"                       
               "mts  $s13 = $r14;"                       
               "mts  $s14 = $r15;"                       
               "mts  $s15 = $r16;"                       
               /* Restore general-purpose registers */   
               "lws  $r3 = [3];"                         
               "lws  $r4 = [4];"                         
               "lws  $r5 = [5];"                         
               "lws  $r6 = [6];"                         
               "lws  $r7 = [7];"                         
               "lws  $r8 = [8];"                         
               "lws  $r9 = [9];"                         
               "lws  $r10 = [10];"                       
               "lws  $r11 = [11];"                       
               "lws  $r12 = [12];"                       
               "lws  $r13 = [13];"                       
               "lws  $r14 = [14];"                       
               "lws  $r15 = [15];"                       
               "lws  $r16 = [16];"                       
               "lws  $r17 = [17];"                       
               "lws  $r18 = [18];"                       
               "lws  $r19 = [19];"                       
               "lws  $r20 = [20];"                       
               "lws  $r21 = [21];"                       
               "lws  $r22 = [22];"                       
               "lws  $r23 = [23];"                       
               "lws  $r24 = [24];"                       
               "lws  $r25 = [25];"                       
               "lws  $r26 = [26];"                       
               "lws  $r27 = [27];"                       
               "lws  $r28 = [28];"                       
               "lws  $r29 = [29];"                       
               "lws  $r30 = [30];"                       
               /* Restore the stack cache state */       
               "lwc  $r1 = [$r31 + 2];"                  
               "sfree 48;"                               
               "sens $r1;"                               
               /* Return to exception base/offset */     
               "xret;"                                   
               "lwc  $r1 = [$r31 + 0];"                  
               "lwc  $r2 = [$r31 + 1];"                  
               "add  $r31 = $r31, 12;"                   
               : :                                       
               /* Clobber everything */                  
               : "$r1", "$r2", "$r3",                    
                 "$r4", "$r5", "$r6", "$r7",             
                 "$r8", "$r9", "$r10", "$r11",           
                 "$r12", "$r13", "$r14", "$r15",         
                 "$r16", "$r17", "$r18", "$r19",         
                 "$r20", "$r21", "$r22", "$r23",         
                 "$r24", "$r25", "$r26", "$r27",         
                 "$r28", "$r29", "$r30", "$r31",         
                 "$s0", "$s1", "$s2", "$s3",             
                 "$s4", "$s5", "$s6", "$s7",             
                 "$s8", "$s9", "$s10", "$s11",           
                 "$s12", "$s13", "$s14", "$s15");
}