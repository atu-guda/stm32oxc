#ifndef _MAIN_H
#define _MAIN_H

#include <span>

#include <oxc_gcode.h>

const inline constinit xfloat M_r2g { 180 / M_PI };
const inline constinit xfloat M_PIx2 { 2 * M_PI };
const inline constinit xfloat M_PI2  { M_PI / 2 };

inline auto& touch_gpio { GpioE };
const uint16_t touch_mask { 0b0100 }; // E2

inline auto& endstops_gpio { GpioD };
const uint16_t endstops_mask { 0b01111011 };

const inline uint32_t  TIM_PWM_base_freq   { 84'000'000 };
const inline uint32_t  TIM_PWM_count_freq  {      10000 };
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
int MX_PWM_common_Init( unsigned idx ); // TODO: channels

void TIM6_callback();

extern int debug; // in main.cpp


class EndStop {
  public:
   EndStop() = default;
   virtual ReturnCode initHW() = 0;
   virtual uint16_t read() = 0;
   uint16_t get() const { return v; }
   virtual bool is_minus_stop() const = 0;
   virtual bool is_minus_go()   const = 0;
   virtual bool is_plus_stop()  const = 0;
   virtual bool is_plus_go()    const = 0;
   virtual bool is_any_stop()   const = 0;
   virtual bool is_clear()      const = 0;
   virtual bool is_bad()        const = 0;
   virtual bool is_clear_for_dir( int dir ) const = 0;
   virtual char toChar() const = 0;
  protected:
   uint16_t v {0};
};

// 2+ pins with positive clear status
class EndStopGpioPos : public EndStop {
  public:
   enum StopBits { minusBit = 0x01, plusBit = 0x02, extraBit = 0x04, mainBits = 0x03 };
   EndStopGpioPos( GpioRegs &a_gi, uint8_t a_start, uint8_t a_n = 2 )
     : pins( a_gi, a_start, a_n, GpioRegs::Pull::down ) {}
   virtual ReturnCode initHW() override { pins.initHW(); return rcOk; };
   virtual uint16_t read() override { v = pins.read(); return v; }
   virtual bool is_minus_stop() const override { return ( ( v & minusBit ) == 0 ) ; }
   virtual bool is_minus_go()   const override { return ( ( v & minusBit ) != 0 ) ; }
   virtual bool is_plus_stop()  const override { return ( ( v & plusBit  ) == 0 ) ; }
   virtual bool is_plus_go()    const override { return ( ( v & plusBit  ) != 0 ) ; }
   virtual bool is_any_stop()   const override { return ( ( v & mainBits ) != mainBits ) ; }
   virtual bool is_clear()      const override { return ( ( v & mainBits ) == mainBits ) ; }
   virtual bool is_bad()        const override { return ( ( v & mainBits ) == 0 ) ; }
   virtual bool is_clear_for_dir( int dir ) const override;
   virtual char toChar() const override {
     static const char es_chars[] { "X+-.??" };
     return es_chars[ v & mainBits];
   }
  protected:
   PinsIn pins;
};


// TODO: base: common props, here - realization
// TODO: ctor, [ptr]
// mach params
class StepMover {
  public:
   enum class EndstopMode { All, Dir, From };
   StepMover( PinsOut *a_motor, EndStop *a_endstops, uint32_t a_tick_2mm, uint32_t a_max_speed, uint32_t a_max_l );
   void initHW();
   void set_dir( int dir );
   ReturnCode step();
   ReturnCode step_dir( int dir ) { set_dir( dir ); return step(); }
   ReturnCode step_to( xfloat to );
   ReturnCode check_es();
   int  get_x() const { return x; }
   int  get_dir() const { return dir; }
   void set_x( int a_x ) { x = a_x; }
   xfloat  get_xf() const { return (xfloat)x / tick2mm; }
   void set_xf( xfloat a_x ) { x = a_x * tick2mm; }
   int  mm2tick( xfloat mm ) { return mm * tick2mm; }
   uint32_t get_max_speed() const { return max_speed; };
   uint32_t get_max_l() const { return max_l; };
   xfloat get_es_find_l() const { return es_find_l; }; // remove?
   xfloat get_k_slow() const { return k_slow; };
   PinsOut* get_motor() { return motor; } // not const, remove?
   EndStop* get_endstops() { return endstops; } // not const, remove? + ask funcs
   void set_es_mode( EndstopMode a_m ) { es_mode = a_m; }
   void set_true_mode( bool m ) { true_mode = m; }
   bool get_true_mode() const { return true_mode; }
  protected:
   PinsOut *motor     ;
   EndStop  *endstops ;
   EndstopMode es_mode { EndstopMode::All };
   uint32_t tick2mm   ; // tick per mm, =  pulses per mm
   uint32_t max_speed ; // mm/min
   uint32_t max_l     ; // mm
   int      x         { 0 }; // current pos in pulses, ? int64_t?
   int      dir       { 0 }; // current direcion: -1, 0, 1, but may be more
   xfloat   es_find_l { 5.0f }; // movement to find endstop, from=ES, to = *1.5
   xfloat   k_slow    { 0.1f }; // slow movement coeff from max_speed
   bool     true_mode { true };
};

const inline constinit unsigned n_motors { 5 }; // TODO: part of Mach

extern StepMover* s_movers[n_motors];

int gcode_cmdline_handler( char *s );
ReturnCode gcode_act_fun_me_st( const GcodeBlock &gc );

// task + state. fill: move_prep_
struct MoveInfo {
  enum class Type { stop = 0, line = 1, circle = 2 }; // TODO: more or remove
  using Act_Pfun = ReturnCode (*)( MoveInfo &mi, xfloat a, xfloat *coo );
  static const unsigned max_params { 10 };
  Type type;
  unsigned n_coo;
  xfloat  p[max_params]; // params itself
  xfloat k_x[max_params]; // coeffs
  xfloat len;
  Act_Pfun step_pfun { nullptr }; // calculate each t
  MoveInfo( MoveInfo::Type tp, unsigned a_n_coo, Act_Pfun pfun );
  bool isGood() const { return step_pfun != nullptr && type != Type::stop ; }
  void zero_arr();
  ReturnCode calc_step( xfloat a, xfloat *coo );
  ReturnCode prep_move_line( const xfloat *prm );
  ReturnCode prep_move_circ( const xfloat *prm );
};

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

   Machine( std::span<StepMover*> a_movers );
   Machine( const Machine &rhs ) = delete;
   void initHW();
   xfloat getPwm() const { return std::clamp( 100 * spin / spin100, 0.0f, spin_max ); }
   int check_endstops( MoveInfo &mi );
   ReturnCode move_common( MoveInfo &mi, xfloat fe_mmm );
   // coords: XYZE....
   ReturnCode move_line( const xfloat *d_mm, xfloat fe_mmm, unsigned a_on_endstop = 9999 );
   // coords: [0]:r_s, [1]: alp_s, [2]: r_e, [3]: alp_e, [4]: cv?, [5]: z_e, [6]: e_e, [7]: nt(L) [8]: x_r, [9]: y_r
   ReturnCode move_circ( const xfloat *d_mm, xfloat fe_mmm );
   ReturnCode go_home( uint16_t motor_bits );
   MachMode get_mode() const { return mode; };
   void set_mode( MachMode m ) { if( m < modeMax ) { mode = m; }}; // TODO: more actions
   xfloat get_xn( unsigned i ) const { return ( i < movers.size() ) ? movers[i]->get_xf() : 0 ; }
   void set_xn( unsigned i, xfloat v ) { if( i < movers.size() ) { movers[i]->set_xf( v ); } }
   int get_dly_xsteps() const { return dly_xsteps; }
   void set_dly_xsteps( int v ) { dly_xsteps = v; }
   unsigned get_n_mo() const { return n_mo; }
   void set_n_mo( unsigned n ) { n_mo = std::min( n_mo, movers.size() ); }
   MoveMode get_move_mode() const { return move_mode; }
   void set_move_mode( MoveMode m ) { move_mode = m; }
   const char* endstops2str( char *buf = nullptr ) const;
   const char* endstops2str_read( char *buf = nullptr );

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
  protected:
   std::span<StepMover*> movers;
   MachMode mode { modeFFF };
   MoveMode move_mode { moveCommon };
   unsigned on_endstop { 9999 };
   uint32_t last_rc;
   unsigned n_mo { 0 }; // current number of active motors
   const FunGcodePair *mg_funcs { nullptr };
   const unsigned mg_funcs_sz;
   xfloat r_min { 0.1f };
   xfloat r_max { 10000.0f };
   xfloat alp_min { M_PI / 180 };
   xfloat near_l { 2.0e-3 };
   xfloat axis_scale[n_motors];
  // public: // for now, TODO: hide
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


void motors_off();
void motors_on();
int wait_next_motor_tick(); // 0 = was break, not 0 - ok
ReturnCode pwm_set( unsigned idx, xfloat v );
ReturnCode pwm_off( unsigned idx );
ReturnCode pwm_off_all();
ReturnCode go_home( unsigned axis );

bool calc_G2_R_mode( bool cv, xfloat x_e, xfloat y_e, xfloat &r_1, xfloat &x_r, xfloat &y_r );



#endif

