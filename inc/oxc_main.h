#ifndef _OXC_MAIN_H
#define _OXC_MAIN_H

// macroses and functions for main.cpp

#include <oxc_base.h>


#ifndef PROLOG_LED_TIME
#define PROLOG_LED_TIME  50
#endif

#ifdef USE_OXC_DEVIO
#define ADD_DEVIO_TICKFUN oxc_add_aux_tick_fun( DevIO::tick_actions_all )
#else
#define ADD_DEVIO_TICKFUN
#endif

#define STD_PROLOG_START \
  HAL_Init(); \
  leds.initHW(); \
  leds.write( BOARD_LEDS_ALL ); \
  int rc = SystemClockCfg(); \
  if( rc ) { \
    die4led( BOARD_LEDS_ALL ); \
    return 1; \
  } \
  ADD_DEVIO_TICKFUN;

#define STD_PROLOG_UART_NOCON \
  STD_PROLOG_START; \
  delay_ms( PROLOG_LED_TIME ); leds.write( 0x00 ); delay_ms( PROLOG_LED_TIME ); \
  if( ! init_uart( &uah_console ) ) { \
    die4led( 1 ); \
  } \
  leds.write( BOARD_LEDS_ALL );  HAL_Delay( PROLOG_LED_TIME );

#define STD_PROLOG_UART \
  STD_PROLOG_UART_NOCON; \
  global_smallrl = &srl; \
  SET_UART_AS_STDIO( dev_console ); \
  std_out.setOut( devio_fds[1] );

#define STD_PROLOG_USBCDC \
  STD_PROLOG_START; \
  delay_ms( PROLOG_LED_TIME ); leds.write( 0x00 ); delay_ms( PROLOG_LED_TIME ); \
  if( ! dev_console.init() ) { \
    die4led( 1 ); \
  } \
  leds.write( BOARD_LEDS_ALL );  HAL_Delay( PROLOG_LED_TIME ); \
  global_smallrl = &srl; \
  SET_USBCDC_AS_STDIO( dev_console ); \
  std_out.setOut( devio_fds[1] );

#ifdef USE_FREERTOS


  //
  #define SCHEDULER_START \
    leds.write( 0x00 ); \
    ready_to_start_scheduler = 1; \
    vTaskStartScheduler(); \
    die4led( 0xFF );
  //
  #define CREATE_STD_TASKS \
    xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr ); \
    xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr ); \
    xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );
    //           code               name    stack_sz      param  prty TaskHandle_t*

#endif


#endif

// vim: path=.,/usr/share/stm32cube/inc
