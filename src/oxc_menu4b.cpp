#include <cstring>
#include <cstdlib>
#include <climits>
#include <iterator>
#include <algorithm>

#include <oxc_menu4b.h>
#include <oxc_outstr.h>
#include <oxc_hd44780_i2c.h>

using namespace std;
using namespace SMLRL;

const char *const MenuState::menuLevelName[] = {
  "Ready <Menu >Run ",
  "Sel:  <Esc >Edit ",
  "Edit: <Esc >Ok ",
  "?3 ", "?4? "
};

volatile uint32_t menu4b_ev_global = 0;

void menu4b_ev_dispatch()
{
  uint32_t ev = menu4b_ev_global;
  menu4b_ev_global = 0;
  if( ev == 0 ) {
    return;
  }

  if( ev == MenuCmd::Run ) { // special case
    // std_out << "#!!! run2" NL;
    ungets( 0, menu4b_state.run_cmd );
    return;
  }

  // TODO: more special cases

  char buf[32];
  buf[0] = 'M'; buf[1] = ' ';
  i2dec_n( ev, buf+2 );
  strcat( buf, "\n" );
  ungets( 0, buf );
}


int menu4b_cmd( int cmd_i )
{
  MenuCmd cmd = MenuCmd( cmd_i );

  const char *item_name = "";

  switch( menu4b_state.level ) {

    case MenuLevel::ready:
      switch( cmd ) {
        case MenuCmd::Esc:
          menu4b_state.level = MenuLevel::select; menu4b_state.index = 0;
          break;
        case MenuCmd::Up:
          break;
        case MenuCmd::Down:
          break;
        case MenuCmd::Enter:
          menu4b_ev_global = MenuCmd::Run;
          delay_ms( 10 );
          break;
        default: break;
      };
      break;

    case MenuLevel::select:
      switch( cmd ) {
        case MenuCmd::Esc:
          menu4b_state.level = MenuLevel::ready;
          break;
        case MenuCmd::Up:
          ++menu4b_state.index;
          break;
        case MenuCmd::Down:
          --menu4b_state.index;
          break;
        case MenuCmd::Enter:
          menu4b_state.level = MenuLevel::edit;
          if( menu4b_state.menu[menu4b_state.index].pv ) {
            menu4b_state.v = *menu4b_state.menu[menu4b_state.index].pv;
          } else {
            menu4b_state.v = menu4b_state.menu[menu4b_state.index].vmin;
          }
          break;
        default: break;
      };
      break;

    case MenuLevel::edit:
      switch( cmd ) {
        case MenuCmd::Esc:
          menu4b_state.level = MenuLevel::select;
          break;
        case MenuCmd::Up:
          menu4b_state.v += menu4b_state.menu[menu4b_state.index].vstep;
          break;
        case MenuCmd::Down:
          menu4b_state.v -= menu4b_state.menu[menu4b_state.index].vstep;
          break;
        case MenuCmd::Enter:
          if( menu4b_state.menu[menu4b_state.index].pv ) {
            *menu4b_state.menu[menu4b_state.index].pv = clamp( menu4b_state.v, menu4b_state.menu[menu4b_state.index].vmin, menu4b_state.menu[menu4b_state.index].vmax );
          }
          if( menu4b_state.menu[menu4b_state.index].fun ) {
            menu4b_state.menu[menu4b_state.index].fun( menu4b_state.v );
          }
          menu4b_state.level = MenuLevel::select;
          break;
        default: break;
      };
      menu4b_state.v = clamp( menu4b_state.v, menu4b_state.menu[menu4b_state.index].vmin, menu4b_state.menu[menu4b_state.index].vmax );
      break;

    default: // ???
      break;
  }

  menu4b_state.index = clamp( menu4b_state.index, 0, (int)(menu4b_state.menu_size-1) );
  item_name = menu4b_state.menu[menu4b_state.index].name;

  int show_v = 0;
  char pre_char = ' ';
  switch( menu4b_state.level ) {
    case MenuLevel::ready:
      break;
    case MenuLevel::select:
      if( menu4b_state.menu[menu4b_state.index].pv ) {
        show_v = *menu4b_state.menu[menu4b_state.index].pv;
      }
      pre_char = '>';
      break;
    case MenuLevel::edit:
      show_v = menu4b_state.v;
      pre_char = '*';
      break;
    default:
      break;
  };


  char buf[32];
  OSTR( b0, 32 );
  buf[0] = pre_char; buf[1] = ' '; buf[2] = '\0';
  if( menu4b_state.level != MenuLevel::ready ) {
    strcat( buf, item_name );
    strcat( buf, "= " );
    if( menu4b_state.menu[menu4b_state.index].div10 == 0 ) {
      b0 << show_v;
    } else {
      b0 << FloatMult( show_v, menu4b_state.menu[menu4b_state.index].div10 );
    }
    strcat( buf, b0.getBuf() );
    strcat( buf, "   " );
  }

  menu4b_output( buf, MenuState::menuLevelName[(int)menu4b_state.level] );

  return 0;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

