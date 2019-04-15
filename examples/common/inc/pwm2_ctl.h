/*
 *    Description:  common declarations for pwm ctl functions (1ch) v2
 */

#ifndef _PWM2_CTL_H
#define _PWM2_CTL_H

//* named indexes in measured data arrays
enum DataIdx {
  didx_v = 0, didx_i = 1, didx_pwm = 2, didx_r = 3, didx_w = 4, didx_val = 5, didx_n
};

//* misct data about pwm. here: fallback values. need calibration
struct PWMInfo {
  float R_0   = 1.0f;      //* initial resistance
  float V_00  = -0.5f;     //* V(0) for linear represenration
  float k_gv1 = 0.12f;     //* dV/d\gamma
  float k_gv2 = 0.006f;    //* a_2 coeff for initial part, = -k_gv1 / (4*V_00)
  float ki_v  = 0.1f;      //* intergation coeff for voltage
  float rehint_lim = 0.2f; //* is rehint needed in calcNextStep
  float V_max = 8.0f;      //* voltage limit
  float I_max = 8.0f;      //* current limit
  float R_max = 8.0f;      //* resistance value for break detection
  float W_max = 90.0f;     //* power limit
  float hint_for_V( float V ) const;
};


class PWMData {
  public:
   enum { max_pwm_steps = 64 };
   enum class pwm_type
   {
     //          1       2     3      4    5
     pwm  = 0, volt,  curr,  pwr,  temp,   n
   };
   enum class check_result {
     ok   = 0,
     soft = 1,
     hard = 2
   };
   struct StepInfo {
     float vb, ve; //* values: begin/end
     int t;        //* time in ms
     pwm_type tp;
   };
   PWMData( PWMInfo &pi, void (*pwm_fun)(float) )
     : pwminfo( pi ), set_pwm_real( pwm_fun )  { reset_steps(); };
   ~PWMData() = default;
   float get_v()      const { return val;   }
   float get_pwm_given() const { return pwm_val; }
   float get_pwm_real() const { return pwm_r; }
   float get_t()  const { return t; }
   int get_c_step()  const { return c_step; }
   void reset_steps();
   void mk_rect( float vmin, float vmax, int t, pwm_type tp );
   void mk_ladder( float v0, float dv, int dt, unsigned n_up, pwm_type tp );
   void mk_ramp( float vmin, float vmax, int t1, int t2, int t3, pwm_type tp );
   void show_steps() const;
   bool edit_step( unsigned ns, float vb, float ve, int t, pwm_type tp );
   void set_pwm();
   float get_pwm_min() const { return pwm_min; }
   float get_pwm_def() const { return pwm_def; }
   float get_pwm_max() const { return pwm_max; }
   void set_pwm_min( float m ) { pwm_min = m; }
   void set_pwm_def( float m ) { pwm_def = std::clamp( pwm_min, m, pwm_max ); }
   void set_pwm_max( float m ) { pwm_max = pwm_tmax = m; }
   void prep( int a_t_step, bool fake );
   bool tick( const float *d ); // returns: true = continue;
   void end_run();
   check_result check_lim( const float *d );
   void set_pwm_manual( float v );
   void set_hand( float v ) { hand = v; }
   void add_to_hand( float v ) { hand += v; }
   void adj_hand_to( float v ) { hand = v - val_1; }
   void set_t_mul( float tmul ) { t_mul = tmul; }
  protected:
   float pwm_min = 3.0f, pwm_def = 5.0f, pwm_max = 60.0f, pwm_tmax = pwm_max; // tmax - tmp max for limits
   float val =  pwm_def;
   float pwm_val =  pwm_def;    // unrestricted
   float val_1 = val;    // w/o hand
   float pwm_r = val;    // real
   float val_0 = val;
   float step_t = 1.0f;  // current step length
   float ks    = 0;      // current step coeff
   float hand = 0;       // adjusted by hand (handle_keys)
   float last_R = 1.0f;

   unsigned n_steps  = 0;
   unsigned c_step   = 0;
   int t = 0;
   int t_step = 10;
   float t_mul = 1;
   bool fake_run = false;
   StepInfo steps[max_pwm_steps];
   PWMInfo &pwminfo;
   void (*set_pwm_real)(float) = nullptr;
   void calcNextStep();
};

extern PWMData pwmdat; // for commands


int cmd_show_steps( int argc, const char * const * argv );
int cmd_mk_rect(    int argc, const char * const * argv );
int cmd_mk_ladder(  int argc, const char * const * argv );
int cmd_mk_ramp(    int argc, const char * const * argv );
int cmd_edit_step(  int argc, const char * const * argv );

extern CmdInfo CMDINFO_SET_MINMAX;
extern CmdInfo CMDINFO_SHOW_STEPS;
extern CmdInfo CMDINFO_MK_RECT;
extern CmdInfo CMDINFO_MK_LADDER;
extern CmdInfo CMDINFO_MK_RAMP;
extern CmdInfo CMDINFO_EDIT_STEP;

#define CMDINFOS_PWM  &CMDINFO_SHOW_STEPS,  &CMDINFO_MK_RECT,  &CMDINFO_MK_LADDER,  &CMDINFO_MK_RAMP, &CMDINFO_EDIT_STEP

#endif

