#include <oxc_gcode.h>
#include "main.h"

#define OUT std_out

// --------- gcodes
// really ms is MachState, but conversion in func prts is impossible
// TODO: all actions in machine, here only args

int gcode_G0G1( const GcodeBlock *cb, MachStateBase *ms, bool g1 )
{
  COMMON_GM_CODE_CHECK;

  // if( ! me_st.was_set ) {
  //   std_out << "# Error: zero point not set" << NL;
  //   return GcodeBlock::rcErr;
  // }

  // TODO: move logic to machine

  const unsigned n_mo { 4 };
  const xfloat meas_scale = me_st.inchUnit ? 25.4f : 1.0f;

  xfloat prev_x[n_mo], d_mm[n_mo];
  for( unsigned i=0; i<n_mo; ++i ) {
    prev_x[i] = me_st.relmove ? 0 : ( me_st.x[i] / meas_scale );
  }

  for( unsigned i=0; i<n_mo; ++i ) {
    d_mm[i] = cb->fpv_or_def( GcodeBlock::axis_chars[i], prev_x[i] );
    d_mm[i] *= meas_scale;
  }

  if( !me_st.relmove ) {
    for( unsigned i=0; i<n_mo; ++i ) {
      d_mm[i] -= prev_x[i];
    }
  }

  xfloat fe_mmm = g1 ? me_st.fe_g1 : me_st.fe_g0;

  OUT << "# G" << (g1?'1':'0') << " ( ";
  for( auto xx: d_mm ) {
    OUT << xx << ' ';
  }
  OUT << " ); fe= "<< fe_mmm << NL;

  if( me_st.get_mode() == MachState::modeLaser && g1 && me_st.spin > 0 ) {
    const xfloat v = me_st.getPwm();
    OUT << "# spin= " << me_st.spin << " v= " << v << NL;
    pwm_set( 0, v );
  }

  int rc = me_st.move_line( d_mm, n_mo, fe_mmm );

  if( me_st.get_mode() == MachState::modeLaser ) {
    pwm_off( 0 );
  }
  OUT << "#  G0G1 rc= "<< rc << " break_flag= " << break_flag << NL;

  return rc == 0 ? GcodeBlock::rcOk : GcodeBlock::rcErr;
}

int gcode_G0( const GcodeBlock *cb, MachStateBase *ms )
{
  return gcode_G0G1( cb, ms, false );
}

int gcode_G1( const GcodeBlock *cb, MachStateBase *ms )
{
  return gcode_G0G1( cb, ms, true );
}

int gcode_G4( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;

  xfloat w = 1000 * cb->fpv_or_def( 'P', 0 );
  w += cb->fpv_or_def( 'S', 1 );
  std_out << "# wait " << w << NL;
  delay_ms_brk( (uint32_t)w );
  return GcodeBlock::rcOk;
}

int gcode_G20( const GcodeBlock *cb, MachStateBase *ms )
{
  me_st.inchUnit = true;
  return GcodeBlock::rcOk;
}

int gcode_G21( const GcodeBlock *cb, MachStateBase *ms )
{
  me_st.inchUnit = false;
  return GcodeBlock::rcOk;
}

int gcode_G28( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;

  bool x = cb->is_set('X'); bool y = cb->is_set('Y'); bool z = cb->is_set('Z');
  if( !x && !y && !z ) {
    x = y = z = true;
  }
  int rc;

  if( y ) {
    rc  = go_home( 1 );
    if( rc != 1 ) {
      return GcodeBlock::rcErr;
    }
  }

  if( x ) {
    rc  = go_home( 0 );
    if( rc != 1 ) {
      return GcodeBlock::rcErr;
    }
  }

  if( z ) {
    rc  = go_home( 2 );
    if( rc != 1 ) {
      return GcodeBlock::rcErr;
    }
  }
  me_st.was_set = true;

  return rc == 1 ? GcodeBlock::rcOk : GcodeBlock::rcErr;
}


int gcode_G90( const GcodeBlock *cb, MachStateBase *ms )
{
  me_st.relmove = false;
  return GcodeBlock::rcOk;
}

int gcode_G91( const GcodeBlock *cb, MachStateBase *ms )
{
  me_st.relmove = true;
  return GcodeBlock::rcOk;
}

int gcode_G92( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;
  bool a { false };
  if( cb->is_set('X') ) {
    me_st.x[0] = cb->fpv_or_def( 'X', 0 ); a = true;
  }
  if( cb->is_set('Y') ) {
    me_st.x[0] = cb->fpv_or_def( 'Y', 0 ); a = true;
  }
  if( cb->is_set('Z') ) {
    me_st.x[0] = cb->fpv_or_def( 'Z', 0 ); a = true;
  }
  if( cb->is_set('E') ) {
    me_st.x[0] = cb->fpv_or_def( 'E', 0 ); a = true;
  }

  if( a ) {
    me_st.was_set = true; // do not change if a false
  }

  return GcodeBlock::rcOk;
}

const MachStateBase::FunGcodePair mach_g_funcs[] {
  {  0, gcode_G0  },
  {  1, gcode_G1  },
  {  4, gcode_G4  },
  { 20, gcode_G20 },
  { 21, gcode_G21 },
  { 28, gcode_G28 },
  { 90, gcode_G90 },
  { 91, gcode_G91 },
  { 92, gcode_G92 },
  { -1, nullptr } //end
};

// ------------------------ M codes -------------------------------------------
int mcode_M0( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;
  OUT << "# M0 " << NL;
  // TODO:
  return GcodeBlock::rcEnd;
}

int mcode_M1( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;

  OUT << "# M1 " << NL;
  // TODO: pause
  return GcodeBlock::rcOk;
}

int mcode_M2( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;
  OUT << "# M2 " << NL;
  // TODO: off all
  pwm_set( 0, 0 );
  motors_off();
  return GcodeBlock::rcEnd;
}

int mcode_M3( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;
  me_st.spin = cb->fpv_or_def( 'S', me_st.spin );
  const xfloat v = me_st.getPwm();
  OUT << "# M3 " << v << NL;
  me_st.spinOn = true;
  if( me_st.get_mode() == MachState::modeCNC ) {
    pwm_set( 0, v ); // no direction
    OUT << '+';
  }
  OUT << NL;
  return GcodeBlock::rcOk;
}

int mcode_M4( const GcodeBlock *cb, MachStateBase *ms )
{
  return mcode_M3( cb, ms );
}

int mcode_M5( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;
  OUT << "# M5 " << NL;
  pwm_off( 0 );
  me_st.spinOn = false;
  return GcodeBlock::rcOk;
}

int mcode_M114( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;
  for( unsigned i=0; i<4; ++i ) { // 4 is XYZE
    OUT << ' ' << GcodeBlock::axis_chars[i] << ": " << ( me_st.x[i] / (me_st.inchUnit ? 25.4f : 1.0f) );
  }
  OUT << NL;
  return GcodeBlock::rcOk;
}

int mcode_M117( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;
  OUT << "# M117" << NL;
  OUT << cb->get_str0() << NL;
  return GcodeBlock::rcOk;
}

int mcode_M220( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;
  me_st.fe_scale = cb->fpv_or_def( 'S', me_st.fe_scale );
  OUT << "# M220 feed scale " << me_st.fe_scale << NL;
  return GcodeBlock::rcOk;
}

int mcode_M221( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;
  me_st.spin100  = cb->fpv_or_def( 'S', me_st.spin100 );
  me_st.spin_max = cb->fpv_or_def( 'U', me_st.spin_max );
  OUT << "# M221 spindle scale " << me_st.spin100 << ' ' << me_st.spin_max << NL;
  return GcodeBlock::rcOk;
}

int mcode_M450( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;
  static const char* const mode_names[] = { "FFF", "Laser", "CNC", "??3", "??4", "??5" };
  OUT << "PrinterMode:" << mode_names[me_st.get_mode()] << NL;
  return GcodeBlock::rcOk;
}

int mcode_M451( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;
  OUT << "# M451 " << NL;
  me_st.set_mode( MachState::modeFFF );
  return GcodeBlock::rcOk;
}

int mcode_M452( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;
  OUT << "# M452 " << NL;
  me_st.set_mode( MachState::modeLaser );
  return GcodeBlock::rcOk;
}

int mcode_M453( const GcodeBlock *cb, MachStateBase *ms )
{
  COMMON_GM_CODE_CHECK;
  OUT << "# M453 " << NL;
  me_st.set_mode( MachState::modeCNC );
  return GcodeBlock::rcOk;
}


const MachStateBase::FunGcodePair mach_m_funcs[] {
  {   0,   mcode_M0 },
  {   1,   mcode_M1 },
  {   2,   mcode_M2 },
  {   3,   mcode_M3 },
  {   4,   mcode_M4 },
  {   5,   mcode_M5 },
  { 114, mcode_M114 },
  { 117, mcode_M117 },
  { 220, mcode_M220 },
  { 221, mcode_M221 },
  { 450, mcode_M450 },
  { 451, mcode_M451 },
  { 452, mcode_M452 },
  { 453, mcode_M453 },
  {  -1, nullptr } //end
};


