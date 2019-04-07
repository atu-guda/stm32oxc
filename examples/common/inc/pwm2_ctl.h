/*
 *    Description:  common declarations for pwm ctl functions (1ch) v2
 */

#ifndef _PWM2_CTL_H
#define _PWM2_CTL_H



class PWMData {
  public:
   enum { max_pwm_steps = 64 };
   enum class pwm_type
   {
     pwm_const  = 0, pwm_lin  = 1,
     volt_const = 2, volt_lin = 3,
     curr_const = 4, curr_lin = 5,
     pwr_const =  6, pwr_lin  = 7,
     temp_const = 8, temp_lin = 9
   };
   struct StepInfo {
     float v; //* value: for *_lin = end level
     int t;   //* time in ms
     pwm_type tp;
   };
   PWMData( TIM_HandleTypeDef &th ) : tim_h( th ) { reset_steps(); };
   ~PWMData() = default;
   float get_v()      const { return val;   }
   float get_v_real() const { return val_r; }
   float get_t()  const { return t; }
   int get_c_step()  const { return c_step; }
   void reset_steps();
   void mk_rect( float v, int t );
   void mk_ladder( float dv, int dt, unsigned n_up );
   void mk_ramp( float v, int t1, int t2, int t3 );
   void show_steps() const;
   bool edit_step( unsigned ns, float v, int t, int tp );
   void set_pwm();
   float get_min() const { return vmin; }
   float get_def() const { return vdef; }
   float get_max() const { return vmax; }
   void set_min( float m ) { vmin = m; }
   void set_def( float m ) { vdef = std::clamp( vmin, m, vmax ); }
   void set_max( float m ) { vmax = m; }
   void prep( int a_t_step, bool fake );
   bool tick(); // returns: true = continue;
   void end_run();
   void set_v_manual( float v );
   void set_hand( float v ) { hand = v; }
   void add_to_hand( float v ) { hand += v; }
   void adj_hand_to( float v ) { hand = v - val_1; }
   void set_t_mul( float tmul ) { t_mul = tmul; }
  protected:
   float vmin = 3.0f, vdef = 5.0f, vmax = 60.0f;
   float val =  vdef;    // unrestricted
   float val_1 = val;    // w/o hand
   float val_r = val;    // real
   float val_0 = val;
   float step_t = 1.0f;  // current step length
   float ks    = 0;      // current step coeff
   float hand = 0;       // adjusted by hand (handle_keys)

   unsigned n_steps  = 0;
   unsigned c_step   = 0;
   int t = 0;
   int t_step = 10;
   float t_mul = 1;
   bool fake_run = false;
   StepInfo steps[max_pwm_steps];
   TIM_HandleTypeDef &tim_h;
   void calcNextStep();
};

extern PWMData pwmdat;


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

