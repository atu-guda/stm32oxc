#ifndef _MAIN_H
#define _MAIN_H

#include <oxc_gcode.h>

const inline uint32_t  TIM_PWM_base_freq   { 84'000'000 };
const inline uint32_t  TIM_PWM_count_freq  {      10000 };
const inline uint32_t  TIM6_base_freq   {  1'000'000 };
const inline uint32_t  TIM6_count_freq  {      10000 };

extern int debug; // in main.cpp

struct MoveInfo;

// mach params
struct MachParam {
  uint32_t tick2mm   ; // tick per mm, = 2* pulses per mm
  uint32_t max_speed ; // mm/min
  uint32_t max_l     ; // mm
  xfloat    es_find_l ; // movement to find endstop, from ES, to = *1.5
  xfloat    k_slow    ; // slow movement coeff from max_speed
  PinsIn  *endstops  ;
  PinsOut *motor     ;
  void set_dir( int dir );
  void step();
};

const inline constinit unsigned n_motors { 5 };

extern MachParam machs[n_motors];

int gcode_cmdline_handler( char *s );
int gcode_act_fun_me_st( const GcodeBlock &gc );

class MachState : public MachStateBase {
  public:
   enum MachMode {
     modeFFF = 0, modeLaser = 1, modeCNC = 2, modeMax = 3
   };
   using fun_gcode_mg_new = int(MachState::*)( const GcodeBlock &cb );
   struct FunGcodePair_new {
     int num;
     fun_gcode_mg_new fun;
   };

   MachState( fun_gcode_mg prep, const FunGcodePair *g_f, const FunGcodePair *m_f );
   xfloat getPwm() const { return std::clamp( 100 * spin / spin100, 0.0f, spin_max ); }
   int check_endstops( MoveInfo &mi );
   int move_common( MoveInfo &mi, xfloat fe_mmm );
   int move_line( const xfloat *d_mm, unsigned n_coo, xfloat fe_mmm, unsigned a_on_endstop = 9999 );
   int move_circ( const xfloat *d_mm, unsigned n_coo, xfloat fe_mmm );
   int step( unsigned i_motor, int dir );
   MachMode get_mode() const { return mode; };
   void set_mode( MachMode m ) { if( m < modeMax ) { mode = m; }};
   xfloat get_xi( unsigned i ) const { return x[i]; }
   xfloat get_x()  const { return x[0]; }
   xfloat get_y()  const { return x[1]; }
   xfloat get_z()  const { return x[2]; }
   xfloat get_e0() const { return x[3]; }
   void set_xi( unsigned i, xfloat v ) { x[i] = v; }
   void set_x(  xfloat v ) { x[0] = v; }
   void set_y(  xfloat v ) { x[1] = v; }
   void set_z(  xfloat v ) { x[2] = v; }
   void set_e0( xfloat v ) { x[3] = v; }
   int get_dly_xsteps() const { return dly_xsteps; }
   void set_dly_xsteps( int v ) { dly_xsteps = v; }
   uint32_t get_n_mo() const { return n_mo; }

   int call_mg_new( const GcodeBlock &cb );

   int g_move_line( const GcodeBlock &gc );     // G0, G1
   int g_move_circle( const GcodeBlock &gc );   // G2, G3
   int g_wait( const GcodeBlock &gc );          // G4
   int g_set_unit_inch( const GcodeBlock &gc ); // G20
   int g_set_unit_mm( const GcodeBlock &gc );   // G21
   int g_home( const GcodeBlock &gc );          // G28
   int g_set_absmove( const GcodeBlock &gc );   // G90
   int g_set_relmove( const GcodeBlock &gc );   // G91
   int g_set_origin( const GcodeBlock &gc );    // G92
   int m_end0( const GcodeBlock &gc );          // M0
   int m_pause( const GcodeBlock &gc );         // M1
   int m_end( const GcodeBlock &gc );           // M2
   int m_set_spin( const GcodeBlock &gc );      // M3, M4
   int m_spin_off( const GcodeBlock &gc );      // M5
   int m_out_where( const GcodeBlock &gc );     // M114
   int m_out_str( const GcodeBlock &gc );       // M117
   int m_set_feed_scale( const GcodeBlock &gc );// M220
   int m_set_spin_scale( const GcodeBlock &gc );// M221
   int m_out_mode( const GcodeBlock &gc );      // M450
   int m_set_mode_fff( const GcodeBlock &gc );  // M451
   int m_set_mode_laser( const GcodeBlock &gc );// M452
   int m_set_mode_cnc( const GcodeBlock &gc );  // M453
  protected:
   MachMode mode { modeFFF };
   int dirs[n_motors];  // last/current direction
   unsigned on_endstop { 9999 };
   uint32_t last_rc;
   uint32_t n_mo { 0 }; // current number of active motors
   const FunGcodePair_new *mg_funcs_new { nullptr };
   const unsigned mg_funcs_new_sz;
  public: // for now, TODO: hide
   xfloat x[n_motors];
   xfloat axis_scale[n_motors];
   xfloat fe_g0 { 350 };
   xfloat fe_g1 { 300 };
   xfloat fe_scale { 100.0f };
   xfloat spin  {   0 };
   xfloat spin100  { 10000 }; // scale for laser PWM
   xfloat spin_max {  90 };   // max PWM in %
   int dly_xsteps { 50 }; // delay between steps in program
   bool was_set { false };
   bool relmove { false };
   bool inchUnit { false };
   bool spinOn   { false };
};

extern MachState me_st;
int mach_prep_fun( const GcodeBlock *cb, MachStateBase *ms );
extern const MachStateBase::FunGcodePair mach_g_funcs[];
extern const MachStateBase::FunGcodePair mach_m_funcs[];
extern const MachState::FunGcodePair_new mg_code_funcs[];


// task + state. fill: move_prep_
struct MoveInfo {
  enum class Type { stop = 0, line = 1, circle = 2 }; // TODO: more
  enum class Ret  { nop = 0, move = 1, end = 2, err = 3 };
  using Act_Pfun = Ret (*)( MoveInfo &mi, xfloat a );
  static const unsigned max_params { 10 };
  Type type;
  unsigned n_coo;
  xfloat  p[max_params]; // params itself,
  xfloat cf[max_params]; // state: floats - unneeded?
  int    ci[max_params]; // state: ints
  xfloat k_x[max_params]; // a[0:1]-based coeffs
  int    cdirs[n_motors]; // current step direction: -1, 0, 1
  xfloat len;
  xfloat t_sec; // approx
  uint32_t t_tick; // in ticks
  Act_Pfun step_pfun { nullptr }; // calculate each t
  MoveInfo( MoveInfo::Type tp, unsigned a_n_coo, Act_Pfun pfun );
  void zero_arr();
  Ret calc_step( xfloat a );
  int prep_move_line( const xfloat *coo, xfloat fe );
  int prep_move_line( const GcodeBlock &gc );
  int prep_move_circ_center( const xfloat *coo, xfloat fe );
  int prep_move_circ_radius( const xfloat *coo, xfloat fe );
};

void motors_off();
void motors_on();
int wait_next_motor_tick(); // 0 = was break, not 0 - ok
int pwm_set( unsigned idx, xfloat v );
int pwm_off( unsigned idx );
int pwm_off_all();
int go_home( unsigned axis );

inline bool is_endstop_minus_stop( uint16_t e ) { return ( (e & 0x01) == 0 ) ; }
inline bool is_endstop_minus_go(   uint16_t e ) { return ( (e & 0x01) != 0 ) ; }
inline bool is_endstop_plus_stop(  uint16_t e ) { return ( (e & 0x02) == 0 ) ; }
inline bool is_endstop_plus_go(    uint16_t e ) { return ( (e & 0x02) != 0 ) ; }
inline bool is_endstop_any_stop(   uint16_t e ) { return ( (e & 0x03) != 3 ) ; }
inline bool is_endstop_clear(      uint16_t e ) { return ( (e & 0x03) == 3 ) ; }
inline bool is_endstop_bad(        uint16_t e ) { return ( (e & 0x03) == 0 ) ; }
bool is_endstop_clear_for_dir( uint16_t e, int dir );

inline auto& endstops_gpio { GpioD };
const uint16_t endstops_mask { 0b01111011 };
inline char endstop2char( uint16_t e )
{
  static const char es_chars[] = "W+-.??";
  return es_chars[ e & 0x03 ];
}

inline auto& touch_gpio { GpioE };
const uint16_t touch_mask { 0b0100 }; // E2

const char* endstops2str( uint16_t es, bool touch, char *buf = nullptr );
const char* endstops2str_a( char *buf = nullptr );


#endif

