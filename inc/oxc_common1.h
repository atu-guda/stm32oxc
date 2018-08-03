#ifndef _OXC_COMMON1
#define _OXC_COMMON1

#include <oxc_devio.h>


// SmallRL storage and config
int smallrl_print( const char *s, int l ) __weak;
int smallrl_exec( const char *s, int l ) __weak;
void smallrl_sigint(void) __weak; // unused?
void sigint(int v) __weak;

void default_main_loop();

extern "C" {

void task_leds( void *prm UNUSED_ARG ) __weak;
void task_gchar( void *prm UNUSED_ARG ) __weak;
void task_main( void *prm UNUSED_ARG );

}

#define SET_UART_AS_STDIO(usartio) \
  usartio.itEnable( UART_IT_RXNE ); \
  usartio.setOnSigInt( sigint ); \
  devio_fds[0] = &usartio; \
  devio_fds[1] = &usartio; \
  devio_fds[2] = &usartio; \
  delay_ms( 10 );

#define SET_USBCDC_AS_STDIO(usbcdc) \
  usbcdc.setOnSigInt( sigint ); \
  devio_fds[0] = &usbcdc;  \
  devio_fds[1] = &usbcdc;  \
  devio_fds[2] = &usbcdc;  \
  delay_ms( 50 );

  /* usbcdc.init(); \ */


#endif

