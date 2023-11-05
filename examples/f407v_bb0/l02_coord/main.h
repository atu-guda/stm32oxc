#ifndef _MAIN_H
#define _MAIN_H

#include <span>

#include <oxc_gcode.h>

#include "endstopgpio.h"
#include "stepmotor.h"
#include "stepmover.h"
#include "moveinfo.h"

#define STEPDIR_X_GPIO     GpioE
#define STEPDIR_X_STARTPIN 8
#define STEPDIR_Y_GPIO     GpioE
#define STEPDIR_Y_STARTPIN 10
#define STEPDIR_Z_GPIO     GpioE
#define STEPDIR_Z_STARTPIN 12
#define STEPDIR_E_GPIO     GpioE
#define STEPDIR_E_STARTPIN 14
#define STEPDIR_V_GPIO     GpioE
#define STEPDIR_V_STARTPIN 0


const inline constinit xfloat M_r2g { 180 / M_PI };
const inline constinit xfloat M_PIx2 { 2 * M_PI };
const inline constinit xfloat M_PI2  { M_PI / 2 };

inline auto& touch_gpio { GpioE };
const uint16_t touch_mask { 0b0100 }; // E2

inline auto& endstops_gpio { GpioD };
const uint16_t endstops_mask { 0b01111011 };

const inline uint32_t  TIM_PWM_base_freq   { 84'000'000 };
const inline uint32_t  TIM_PWM_count_freq  {       5000 };
const inline uint32_t  TIM6_base_freq      {  1'000'000 };
const inline uint32_t  TIM6_count_freq     {      10000 };


extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;

inline const unsigned n_tim_pwm { 3 };
extern TIM_HandleTypeDef* pwm_tims[n_tim_pwm];

int MX_TIM2_Init();
int MX_TIM3_Init();  // PMW0
int MX_TIM4_Init();
int MX_TIM6_Init();  // tick clock for move
int MX_TIM10_Init(); // PWM1
int MX_TIM11_Init(); // PWM2
void HAL_TIM_MspPostInit( TIM_HandleTypeDef* timHandle );
int MX_PWM_common_Init( unsigned idx, unsigned channel );

void TIM6_callback();

extern int debug; // in main.cpp

const inline constinit unsigned n_motors { 5 }; // TODO: part of Mach

extern StepMover* s_movers[n_motors];

int gcode_cmdline_handler( char *s );
ReturnCode gcode_act_fun_me_st( const GcodeBlock &gc );

// task + state. fill: move_prep_
// struct, as we must have method to relize any move without subclassing?

class Machine {
  public:
   enum MachMode {
     modeFFF = 0, modeLaser = 1, modeCNC = 2, modeMax = 3
   };
   enum MoveMode { // flags
     moveCommon = 0, moveActive = 1, moveFast = 2, moveAllStop = 4
   };

   using fun_gcode_mg = ReturnCode(Machine::*)( const GcodeBlock &cb );
   struct FunGcodePair {
     int num;
     fun_gcode_mg fun;
     const char* helpstr;
   };

   struct VarInfo {
     const char *const name;
     xfloat Machine::*const fptr;
     int    Machine::*const iptr;
   };

   Machine( std::span<StepMover*> a_movers );
   Machine( const Machine &rhs ) = delete;
   void initHW();
   xfloat getPwm() const { return std::clamp( 100 * spin / spin100, 0.0f, spin_max ); }
   ReturnCode move_common( MoveInfo &mi, xfloat fe_mmm );
   // coords: XYZE....
   ReturnCode move_line( const xfloat *prm, xfloat fe_mmm );
   // coords: [0]:r_s, [1]: alp_s, [2]: r_e, [3]: alp_e, [4]: cv?, [5]: z_e, [6]: e_e, [7]: nt(L) [8]: x_r, [9]: y_r
   ReturnCode move_circ( const xfloat *prm, xfloat fe_mmm );
   ReturnCode go_home( unsigned motor_bits );
   ReturnCode go_from_es( unsigned mover_idx );
   MachMode get_mode() const { return mode; };
   void set_mode( MachMode m ) { if( m < modeMax ) { mode = m; }}; // TODO: more actions
   xfloat get_xn( unsigned i ) const { return ( i < movers.size() ) ? movers[i]->get_xf() : 0 ; }
   void set_xn( unsigned i, xfloat v ) { if( i < movers.size() ) { movers[i]->set_xf( v ); } }
   int get_dly_xsteps() const { return dly_xsteps; }
   void set_dly_xsteps( int v ) { dly_xsteps = v; }
   unsigned get_n_mo() const { return n_mo; }
   void set_n_mo( unsigned n ) { n_mo = std::min( n, movers.size() ); active_movers_bits = ~0u >> (sizeof(unsigned)-n_mo); }
   MoveMode get_move_mode() const { return move_mode; }
   void set_move_mode( MoveMode m ) { move_mode = m; }
   const char* endstops2str( char *buf = nullptr ) const;
   const char* endstops2str_read( char *buf = nullptr );
   ReturnCode set_val( const char *name, xfloat v );
   xfloat get_val( const char *name ) const ; // NAN if not found
   void   out_vals() const; // NAN if not found

   ReturnCode call_mg( const GcodeBlock &cb );
   ReturnCode prep_fun(  const GcodeBlock &cb );
   void out_mg( bool is_m );

   ReturnCode g_move_line( const GcodeBlock &gc );     // G0, G1
   ReturnCode g_move_circle( const GcodeBlock &gc );   // G2, G3
   ReturnCode g_wait( const GcodeBlock &gc );          // G4
   ReturnCode g_set_plane( const GcodeBlock &gc );     // G17 - X
   ReturnCode g_set_unit_inch( const GcodeBlock &gc ); // G20
   ReturnCode g_set_unit_mm( const GcodeBlock &gc );   // G21
   ReturnCode g_home( const GcodeBlock &gc );          // G28
   ReturnCode g_off_compens( const GcodeBlock &gc );   // G40
   ReturnCode g_set_absmove( const GcodeBlock &gc );   // G90
   ReturnCode g_set_relmove( const GcodeBlock &gc );   // G91
   ReturnCode g_set_origin( const GcodeBlock &gc );    // G92
   ReturnCode m_end0( const GcodeBlock &gc );          // M0
   ReturnCode m_pause( const GcodeBlock &gc );         // M1
   ReturnCode m_end( const GcodeBlock &gc );           // M2
   ReturnCode m_set_spin( const GcodeBlock &gc );      // M3, M4
   ReturnCode m_spin_off( const GcodeBlock &gc );      // M5
   ReturnCode m_out_where( const GcodeBlock &gc );     // M114
   ReturnCode m_out_str( const GcodeBlock &gc );       // M117
   ReturnCode m_set_feed_scale( const GcodeBlock &gc );// M220
   ReturnCode m_set_spin_scale( const GcodeBlock &gc );// M221
   ReturnCode m_out_mode( const GcodeBlock &gc );      // M450
   ReturnCode m_set_mode_fff( const GcodeBlock &gc );  // M451
   ReturnCode m_set_mode_laser( const GcodeBlock &gc );// M452
   ReturnCode m_set_mode_cnc( const GcodeBlock &gc );  // M453
   // local codes
   ReturnCode m_list_vars( const GcodeBlock &gc );     // M995
   ReturnCode m_set_var(   const GcodeBlock &gc );     // M996 name=S, value = V
   ReturnCode m_get_var(   const GcodeBlock &gc );     // M997 name=S

   static const char* get_axis_chars() { return axis_chars; }
  protected:
   ReturnCode go_from_es_nc( unsigned mover_idx, StepMover *mover, EndStop *es );
   ReturnCode go_to_es_nc(   unsigned mover_idx, StepMover *mover, EndStop *es );

   std::span<StepMover*> movers;
   MachMode mode { modeFFF };
   MoveMode move_mode { moveCommon };
   unsigned n_mo { 0 }; // current number of active motors
   unsigned active_movers_bits { 0 }; // some motors may be ignored at some actions
   const FunGcodePair *mg_funcs { nullptr };
   const unsigned mg_funcs_sz;
   xfloat r_min { 0.1f };
   xfloat r_max { 10000.0f };
   xfloat alp_min { M_PI / 180 };
   xfloat near_l { 2.0e-3 };
   xfloat axis_scale[n_motors];
   xfloat fe_min { 2.0f };
   xfloat fe_g0 { 350 };
   xfloat fe_g1 { 300 };
   xfloat fe_scale { 100.0f };
   xfloat spin  {   0 };
   xfloat spin100  { 10000 }; // scale for laser PWM
   xfloat spin_max {  90 };   // max PWM in %
   int dly_xsteps { 10 }; // delay between steps in program, ms
   int reen_motors { 0 }; // disable/enable motors after each move
   bool bounded_move { true };
   bool relmove { false };
   bool inchUnit { false };
   bool spinOn   { false };
   static const char axis_chars[];
   static const VarInfo var_info[];
};

extern Machine me_st;
extern const Machine::FunGcodePair mg_code_funcs[];


void motors_off();
void motors_on();
int wait_next_motor_tick(); // 0 = was break, not 0 - ok
ReturnCode pwm_set( unsigned idx, xfloat v );
ReturnCode pwm_off( unsigned idx );
ReturnCode pwm_off_all();

bool calc_G2_R_mode( bool cv, xfloat x_e, xfloat y_e, xfloat &r_1, xfloat &x_r, xfloat &y_r );



#endif

