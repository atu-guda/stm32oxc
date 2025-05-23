#include <errno.h>

#include <oxc_auto.h>
#include <oxc_main.h>

#include <oxc_modbus_rd6006.h>

#include <cstring>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


const char* common_help_string = "Appication to test RD6006 PSU via MODBUS RTU server" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_init( int argc, const char * const * argv );
CmdInfo CMDINFO_INIT { "init", '\0', cmd_init, " - init RD6006"  };
int cmd_writeReg( int argc, const char * const * argv );
CmdInfo CMDINFO_WRITEREG { "write_reg", 'W', cmd_writeReg, "reg val - write 1 reg"  };
int cmd_readRegs( int argc, const char * const * argv );
CmdInfo CMDINFO_READREGS { "read_regs", 'R', cmd_readRegs, "start n - read n regs"  };
int cmd_readReg( int argc, const char * const * argv );
CmdInfo CMDINFO_READREG { "read_reg", '\0', cmd_readReg, "i - read 1 reg"  };
int cmd_on( int argc, const char * const * argv );
CmdInfo CMDINFO_ON { "on", '\0', cmd_on, "- set ON"  };
int cmd_off( int argc, const char * const * argv );
CmdInfo CMDINFO_OFF { "off", '\0', cmd_off, "- set OFF"  };
int cmd_measure( int argc, const char * const * argv );
CmdInfo CMDINFO_MEASURE { "measure", 'M', cmd_measure, "- measure "  };
int cmd_setV( int argc, const char * const * argv );
CmdInfo CMDINFO_SETV { "setV", 'V', cmd_setV, "mV [r] - set output voltage "  };
int cmd_setI( int argc, const char * const * argv );
CmdInfo CMDINFO_SETI { "setI", 'I', cmd_setI, "100uA  [r]- set output current "  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_INIT,
  &CMDINFO_WRITEREG,
  &CMDINFO_READREGS,
  &CMDINFO_READREG,
  &CMDINFO_ON,
  &CMDINFO_OFF,
  &CMDINFO_MEASURE,
  &CMDINFO_SETV,
  &CMDINFO_SETI,
  nullptr
};

extern UART_HandleTypeDef huart_modbus;
MODBUS_RTU_server m_srv( &huart_modbus );
RD6006_Modbus rd( m_srv );

void idle_main_task()
{
  // leds.toggle( 1 );
}




int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 2000;
  UVAR('l') =    1; // break measurement if CC mode
  UVAR('n') =   10;
  UVAR('u') =    2; // default unit addr

  UVAR('e') = MX_MODBUS_UART_Init();

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n  = arg2long_d( 1, argc, argv,  UVAR('n'), 0 );
  int v0 = arg2long_d( 2, argc, argv,  0, 0, 50000 );
  int dv = arg2long_d( 3, argc, argv, 10, 0, 10000 );
  uint32_t t_step = UVAR('t');
  std_out <<  "# Test0: n= " << n << " t= " << t_step
          << " v0= " << v0 << " dv= " << dv << NL;

  uint32_t scale = rd.getScale();
  if( ! scale ) {
    std_out << "# Error: scale = 0. init? " NL;
    return 2;
  }

  break_flag = 0;
  auto v_set = v0;
  rd.on();
  delay_ms_brk( t_step );

  for( int i=0; i<n && !break_flag; ++i ) {
    ReturnCode rc = rd.setV( v_set );
    if( rc != rcOk ) {
      std_out << "# setV error: " << rc << NL;
      break;
    }

    if( delay_ms_brk( t_step ) ) {
      break;
    }
    rc = rd.readMain();
    if( rc != rcOk ) {
      std_out << "# readMain error: " << rc << NL;
      break;
    }
    auto err = rd.get_Err();
    auto cc  = rd.get_CC();
    uint32_t V = rd.getV_mV();
    uint32_t I = rd.getI_100uA();
    std_out << v_set << ' ' << V  << ' ' << I << ' ' << ' ' << cc << ' ' << err << NL;
    if( err || ( cc && UVAR('l') && v_set > 80 ) ) { // 80 is mear minial v/o fake CC
      break;
    }

    v_set += dv;
  }

  break_flag = 0;
  std_out << "# prepare to OFF: " ;
  delay_ms( 200 );
  std_out << rd.off() << NL;

  return 0;
}

int cmd_init( int argc, const char * const * argv )
{
  uint8_t addr = arg2long_d( 1, argc, argv, UVAR('u'), 0, 0xFFFF );
  std_out <<  "#  init: addr=" << (int)addr  << NL;
  rd.setAddr( addr );
  auto rc = rd.init();
  std_out << "# scale= " << rd.getScale() << " rc= " << rc << NL;

  return 0;
}

int cmd_on( int argc, const char * const * argv )
{
  auto rc = rd.on();
  std_out << "# ON.  rc= " << rc << NL;
  return 0;
}

int cmd_off( int argc, const char * const * argv )
{
  auto rc = rd.off();
  std_out << "# OFF. rc= " << rc << NL;
  return 0;
}

int cmd_measure( int argc, const char * const * argv )
{
  uint32_t scale = rd.getScale();
  if( ! scale ) {
    std_out << " measure Error: scale = 0 " NL;
    return 2;
  }
  auto err = rd.readErr();
  auto [v,i] = rd.read_VI();
  std_out << v << ' ' << i << ' ' << scale << ' ' << err << NL;
  return 0;
}

int cmd_setV( int argc, const char * const * argv )
{
  uint32_t V = arg2long_d( 1, argc, argv, 0, 0, 60000 );
  int     me = arg2long_d( 2, argc, argv, 0, 0, 1 );
  auto rc = rd.setV( V );
  std_out << "# setV " << V << ' ' << rc << NL;
  if( me ) {
    delay_ms( 1000 );
    auto err = rd.readErr();
    auto [v,i] = rd.read_VI();
    std_out << v << ' ' << i << ' ' << err << NL;
  }
  return 0;
}

int cmd_setI( int argc, const char * const * argv )
{
  uint32_t I = arg2long_d( 1, argc, argv, 0, 0, 50000 );
  auto rc = rd.setI( I );
  delay_ms( 100 );
  std_out << "# setI " << I << ' ' << rc << NL;
  return 0;
}

// keep for debug

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

