#include <oxc_auto.h>

//* 2-string output function: provided by application
int menu4b_output( const char *s1, const char *s2 = nullptr );

extern volatile uint32_t menu4b_ev_global;
void menu4b_ev_dispatch();
int menu4b_cmd( int cmd_i );
int init_menu4b_buttons(); // board dependent function: in separate file

struct Menu4bItem {
  const char *name;
  int *pv;
  int vstep = 1, vmin = 0x80000000, vmax = 0x7FFFFFFF;
  int (*fun)(int) = nullptr;
  int div10 = 0;
};

enum MenuCmd : uint32_t {
  Esc = 1, Up = 2, Down = 3, Enter = 4, Run = 10
};

enum MenuLevel {
  ready = 0, select = 1, edit = 2
};

struct MenuState {
  const Menu4bItem* menu;
  const uint32_t menu_size;
  const char* run_cmd = "h\n";
  MenuLevel level = MenuLevel::ready;
  int index = 0; // selected element index
  int *pv   = nullptr; // ptr current var under edit (level2)
  int v = 0; // current value
  static const char *const menuLevelName[5];
  const char *menu_level0_str = "Ready <Menu >Run";
};

extern MenuState menu4b_state; // usualy in main.cpp

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

