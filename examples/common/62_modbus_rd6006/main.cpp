#include <cstring>
#include <cerrno>

#include <oxc_auto.h>
#include <oxc_main.h>

#include <oxc_modbus_rd6006.h>


using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


const char* common_help_string = "Appication to test RD6006 PSU via MODBUS RTU server" NL;

// --- local commands;
DCL_CMD_REG( test0, 'T', " - test something 0"  );
DCL_CMD_REG( init, '\0', " - init RD6006"  );
DCL_CMD_REG( write_reg, 'W', "reg val - write 1 reg"  );
DCL_CMD_REG( read_regs, 'R', "start n - read n regs"  );
DCL_CMD_REG( read_reg, '\0', "i - read 1 reg"  );
DCL_CMD_REG( on, '\0', "- set ON"  );
DCL_CMD_REG( off, '\0', "- set OFF"  );
DCL_CMD_REG( measure, 'M', "- measure "  );
DCL_CMD_REG( setV, 'V', "mV [r] - set output voltage "  );
DCL_CMD_REG( setI, 'I', "100uA  [r]- set output current "  );


extern UART_HandleTypeDef huart_modbus;
MODBUS_RTU_server m_srv( &huart_modbus );
RD6006_Modbus rd( m_srv );

void idle_main_task()
{
  // leds[1].toggle();
}




int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 2000;
  UVAR_l =    1; // break measurement if CC mode
  UVAR_n =   10;
  UVAR_u =    2; // default unit addr

  UVAR_e = MX_MODBUS_UART_Init();

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n  = arg2long_d( 1, argc, argv,  UVAR_n, 0 );
  int v0 = arg2long_d( 2, argc, argv,  0, 0, 50000 );
  int dv = arg2long_d( 3, argc, argv, 10, 0, 10000 );
  uint32_t t_step = UVAR_t;
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
    if( err || ( cc && UVAR_l && v_set > 80 ) ) { // 80 is mear minial v/o fake CC
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
  uint8_t addr = arg2long_d( 1, argc, argv, UVAR_u, 0, 0xFFFF );
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

int cmd_write_reg( int argc, const char * const * argv )
{
  uint16_t reg = arg2long_d( 1, argc, argv, 0, 0, 0xFFFF );
  uint16_t val = arg2long_d( 2, argc, argv, 0, 0, 0xFFFF );

  std_out <<  "# write1reg :  " << reg << ' ' << val << ' ' << UVAR_u << NL;
  auto rc = m_srv.writeReg( UVAR_u, reg, val );
  std_out << "# rc " << rc << ' ' << m_srv.getError() << ' ' << m_srv.getReplError() << NL;
  return rc;
}

int cmd_read_regs( int argc, const char * const * argv )
{
  uint16_t start = arg2long_d( 1, argc, argv, 0, 0, 0xFFFF );
  uint16_t n     = arg2long_d( 2, argc, argv, 1, 1, 125 );

  std_out <<  "# readNRegs :  " << start << ' ' << n << ' ' << UVAR_u << NL;
  auto rc = m_srv.readRegs( UVAR_u, start, n );

  std_out << "# rc " << rc << ' ' << m_srv.getError() << ' ' << m_srv.getReplError() << NL;

  if( rc == rcOk ) {
    for( uint16_t i=start; i<start+n; ++i ) {
      auto v = m_srv.getReg( i );
      std_out << "# " << HexInt16(i) << ' ' << HexInt16(v) << ' ' << v << NL;
    }
  }
  return rc;
}


int cmd_read_reg( int argc, const char * const * argv )
{
  uint16_t i = arg2long_d( 1, argc, argv, 0, 0, 0xFFFF );

  std_out <<  "# readNReg :  " << i << UVAR_u << NL;
  auto v = m_srv.readGetReg( UVAR_u, i );

  if( v ) {
    std_out << "# v= "  << HexInt16(v.value()) << ' ' << v.value() << NL;
  } else {
    std_out << "# rc " << v.error() << ' ' << m_srv.getError() << ' ' << m_srv.getReplError() << NL;
  }

  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

