#include <errno.h>

#include <oxc_auto.h>

#include <oxc_modbus_rtu_server.h>

#include <cstring>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


const char* common_help_string = "Appication to test MODBUS RTU server" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_out( int argc, const char * const * argv );
CmdInfo CMDINFO_OUT { "out", 'O', cmd_out, " - print and clear ibuf"  };
int cmd_writeReg( int argc, const char * const * argv );
CmdInfo CMDINFO_WRITEREG { "write_reg", 'W', cmd_writeReg, "reg val - write 1 reg"  };
int cmd_readRegs( int argc, const char * const * argv );
CmdInfo CMDINFO_READREGS { "read_regs", 'R', cmd_readRegs, "start n - read n regs"  };
int cmd_readReg( int argc, const char * const * argv );
CmdInfo CMDINFO_READREG { "read_reg", '\0', cmd_readReg, "i - read 1 reg"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_OUT,
  &CMDINFO_WRITEREG,
  &CMDINFO_READREGS,
  &CMDINFO_READREG,
  nullptr
};

extern UART_HandleTypeDef huart_modbus;
MODBUS_RTU_server m_srv( &huart_modbus );

void idle_main_task()
{
  // leds.toggle( 1 );
}




int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100;
  UVAR('n') =   4;
  UVAR('u') =   2; // default unit addr
  UVAR('a') =  '@';

  UVAR('e') = MX_MODBUS_UART_Init();

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  std_out <<  "# Test0: n= " << n << " t= " << t_step << NL;

  uint32_t tm0 = HAL_GetTick();

  uint32_t tc0 = tm0, tc00 = tm0;
  uint16_t  txx0 = TIM11->CNT;


  uint32_t tmc_prev = tc0;
  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    uint32_t  tcc = HAL_GetTick();
    uint16_t  txx1 = TIM11->CNT;
    uint16_t  txx2 = TIM11->CNT;
    if( i == 0 ) {
      txx0 = txx1;
    }
    uint16_t txx_d = txx1 - txx0;
    std_out <<  "i= " << i << "  tick= " << ( tcc - tc00 )
            << " dlt= " << ( tcc - tmc_prev ) << ' '
            << txx1 << ' ' << txx2 << ' ' << txx_d << NL;
    tmc_prev = tcc;
    txx0 = txx1;

    UART_MODBUS->USART_TX_REG = (uint8_t)UVAR('a');

    delay_ms_until_brk( &tc0, t_step );
  }

  return 0;
}

int cmd_out( int argc, const char * const * argv )
{
  std_out <<  "#  " << NL;

  dump8( m_srv.get_ibuf(), 0x40 );

  m_srv.reset();
  return 0;
}

int cmd_writeReg( int argc, const char * const * argv )
{
  uint16_t reg = arg2long_d( 1, argc, argv, 0, 0, 0xFFFF );
  uint16_t val = arg2long_d( 2, argc, argv, 0, 0, 0xFFFF );

  std_out <<  "# write1reg :  " << reg << ' ' << val << ' ' << UVAR('u') << NL;
  auto rc = m_srv.writeReg( UVAR('u'), reg, val );
  std_out << "# rc " << rc << ' ' << m_srv.getError() << ' ' << m_srv.getReplError() << NL;
  return rc;
}

int cmd_readRegs( int argc, const char * const * argv )
{
  uint16_t start = arg2long_d( 1, argc, argv, 0, 0, 0xFFFF );
  uint16_t n     = arg2long_d( 2, argc, argv, 1, 1, 125 );

  std_out <<  "# readNRegs :  " << start << ' ' << n << ' ' << UVAR('u') << NL;
  auto rc = m_srv.readRegs( UVAR('u'), start, n );

  std_out << "# rc " << rc << ' ' << m_srv.getError() << ' ' << m_srv.getReplError() << NL;

  if( rc == rcOk ) {
    for( uint16_t i=start; i<start+n; ++i ) {
      auto v = m_srv.getReg( i );
      std_out << "# " << HexInt16(i) << ' ' << HexInt16(v) << ' ' << v << NL;
    }
  }
  return rc;
}


int cmd_readReg( int argc, const char * const * argv )
{
  uint16_t i = arg2long_d( 1, argc, argv, 0, 0, 0xFFFF );

  std_out <<  "# readNReg :  " << i << UVAR('u') << NL;
  auto v = m_srv.readGetReg( UVAR('u'), i );

  if( v ) {
    std_out << "# v= "  << HexInt16(v.value()) << ' ' << v.value() << NL;
  } else {
    std_out << "# rc " << v.error() << ' ' << m_srv.getError() << ' ' << m_srv.getReplError() << NL;
  }

  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

