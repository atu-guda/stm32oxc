#ifndef _OXC_AT_DIS_IRQ_H
#define _OXC_AT_DIS_IRQ_H

#include <oxc_asm.h>

// only from C++

template <typename F, typename... Args > auto at_disabled_irq( F fun, Args&&... args )
{
  bool was_irqs_enabled = ( __get_PRIMASK() == 0 );
  oxc_disable_interrupts();
  if constexpr( std::is_void_v<decltype( fun(std::forward<Args>(args)...) )>) {
    fun( std::forward<Args>(args)... );
    if( was_irqs_enabled ) {
      oxc_enable_interrupts();
    }
  } else {
    auto res = fun( std::forward<Args>(args)... );
    if( was_irqs_enabled ) {
      oxc_enable_interrupts();
    }
    return res;
  }
}

#endif

