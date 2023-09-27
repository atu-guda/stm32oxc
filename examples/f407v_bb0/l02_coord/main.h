#ifndef _MAIN_H
#define _MAIN_H

#include <oxc_gcode.h>

const inline uint32_t  TIM_PWM_base_freq   { 84'000'000 };
const inline uint32_t  TIM_PWM_count_freq  {      10000 };
const inline uint32_t  TIM6_base_freq   {  1'000'000 };
const inline uint32_t  TIM6_count_freq  {      10000 };

extern int debug; // in main.cpp

struct MoveInfo;

// TODO: base: common props, here - realization
// mach params
struct StepMover {
  uint32_t tick2mm   ; // tick per mm, =  pulses per mm
  int      x         ; // current pos in pulses, ? int64_t?
  int      dir       ; // current direcion: -1, 0, 1, but may be more
  uint32_t max_speed ; // mm/min
  uint32_t max_l     ; // mm
  xfloat    es_find_l ; // movement to find endstop, from=ES, to = *1.5
  xfloat    k_slow    ; // slow movement coeff from max_speed
  PinsIn  *endstops  ;
  PinsOut *motor     ;
  void set_dir( int dir );
  void step();
  void step_dir( int dir ) { set_dir( dir ); step(); }
  int  get_x() const { return x; }
  void set_x( int a_x ) { x = a_x; }
  xfloat  get_xf() const { return (xfloat)x / tick2mm; }
  void set_xf( xfloat a_x ) { x = a_x * tick2mm; }
  int  mm2tick( xfloat mm ) { return mm * tick2mm; }
};

const inline constinit unsigned n_motors { 5 }; // TODO: part of Mach

extern StepMover s_movers[n_motors];

int gcode_cmdline_handler( char *s );
int gcode_act_fun_me_st( const GcodeBlock &gc );

class Machine {
  public:
   enum MachMode {
     modeFFF = 0, modeLaser = 1, modeCNC = 2, modeMax = 3
   };
   using fun_gcode_mg = int(Machine::*)( const GcodeBlock &cb );
   struct FunGcodePair {
     int num;
     fun_gcode_mg fun;
     const char* helpstr;
   };

   Machine( StepMover *a_movers, unsigned a_n_movers );
   xfloat getPwm() const { return std::clamp( 100 * spin / spin100, 0.0f, spin_max ); }
   int check_endstops( MoveInfo &mi );
   int move_common( MoveInfo &mi, xfloat fe_mmm );
   // coords: XYZE....
   int move_line( const xfloat *d_mm, unsigned n_coo, xfloat fe_mmm, unsigned a_on_endstop = 9999 );
   // coords: [0]:r_s, [1]: alp_s, [2]: r_e, [3]: alp_e, [4]: cv?, [5]: z_e, [6]: e_e, [7]: nt(L) [8]: x_r, [9]: y_r
   int move_circ( const xfloat *d_mm, unsigned n_coo, xfloat fe_mmm );
   int step( unsigned i_motor, int dir );
   MachMode get_mode() const { return mode; };
   void set_mode( MachMode m ) { if( m < modeMax ) { mode = m; }};
   xfloat get_xn( unsigned i ) const { return ( i < n_movers ) ? movers[i].get_xf() : 0 ; }
   void set_xn( unsigned i, xfloat v ) { if( i < n_movers ) { movers[i].set_xf( v ); } }
   int get_dly_xsteps() const { return dly_xsteps; }
   void set_dly_xsteps( int v ) { dly_xsteps = v; }
   uint32_t get_n_mo() const { return n_mo; }

   int call_mg( const GcodeBlock &cb );
   void out_mg( bool is_m );
   int prep_fun(  const GcodeBlock &cb );

   int g_move_line( const GcodeBlock &gc );     // G0, G1
   int g_move_circle( const GcodeBlock &gc );   // G2, G3
   int g_wait( const GcodeBlock &gc );          // G4
   int g_set_plane( const GcodeBlock &gc );     // G17 - X
   int g_set_unit_inch( const GcodeBlock &gc ); // G20
   int g_set_unit_mm( const GcodeBlock &gc );   // G21
   int g_home( const GcodeBlock &gc );          // G28
   int g_off_compens( const GcodeBlock &gc );   // G40
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
   StepMover* movers; // TODO: common mover
   const unsigned n_movers;
   MachMode mode { modeFFF };
   unsigned on_endstop { 9999 };
   uint32_t last_rc;
   uint32_t n_mo { 0 }; // current number of active motors
   const FunGcodePair *mg_funcs { nullptr };
   const unsigned mg_funcs_sz;
   xfloat r_min { 0.1f };
   xfloat r_max { 10000.0f };
  public: // for now, TODO: hide
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

extern Machine me_st;
extern const Machine::FunGcodePair mg_code_funcs[];


// task + state. fill: move_prep_
struct MoveInfo {
  enum class Type { stop = 0, line = 1, circle = 2 }; // TODO: more
  enum class Ret  { nop = 0, move = 1, end = 2, err = 3 };
  using Act_Pfun = Ret (*)( MoveInfo &mi, xfloat a );
  static const unsigned max_params { 10 };
  Type type;
  unsigned n_coo;
  xfloat  p[max_params]; // params itself
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
  int prep_move_circ( const xfloat *coo, xfloat fe );
};

void motors_off();
void motors_on();
int wait_next_motor_tick(); // 0 = was break, not 0 - ok
int pwm_set( unsigned idx, xfloat v );
int pwm_off( unsigned idx );
int pwm_off_all();
int go_home( unsigned axis );

bool calc_G2_R_mode( bool cv, xfloat x_e, xfloat y_e, xfloat &r_1, xfloat &x_r, xfloat &y_r );

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

