#ifndef _OXC_ASM_H
#define _OXC_ASM_H


#ifdef __cplusplus
 extern "C" {
#endif

inline void oxc_enable_interrupts(void)
{
  __asm__ volatile ( "CPSIE I" : : : "memory");
}

inline void oxc_disable_interrupts(void)
{
  __asm__ volatile ( "CPSID I" : : : "memory" );
}


inline void oxc_dmb(void)
{
  __asm__ volatile ( "DMB 0xF" : : : "memory" );
}

inline uint32_t oxc_ldrex( volatile uint32_t *addr )
{
  uint32_t rv;
  __asm__ volatile ( "ldrex %0, [%1]" : "=r" (rv) : "r" (addr) );
  return rv;
}

inline uint32_t oxc_strex( uint32_t val, volatile uint32_t *addr )
{
  uint32_t rv;
  __asm__ volatile ( "strex %0, %2, [%1]"
      : "=&r" (rv) : "r" (addr), "r" (val) );
  return rv;
}



#ifdef __cplusplus
}
#endif


#endif

