#ifndef _MAIN_H
#define _MAIN_H

extern int debug;
extern int dry_run;
extern int dis_movers;
extern int adc_n;
bool is_mover_disabled( unsigned ch, bool do_print = false );
int bad_coord_idx(); // < 0 = ok
bool is_good_coords( bool do_stop = true, bool do_print = false, bool do_measure = false );



inline auto& LEDSX_GPIO { GpioB };
inline constexpr uint32_t LEDSX_START { 12 };
inline constexpr uint32_t LEDSX_N { 4 };

inline auto& BTN_STOP_GPIO                  { GpioB };
inline constexpr uint32_t BTN_STOP_PIN      { 12 };
inline constexpr uint16_t BTN_STOP_IRQ_PRTY {  3 };
inline constexpr auto     BTN_STOP_IRQ_N    { EXTI2_IRQn };
inline constexpr auto     BTN_STOP_EXTI_DIR { GpioRegs::ExtiEv::updown };
void init_EXTI();

inline auto& LWM_GPIO { GpioA };
inline constexpr uint32_t LWM_PIN0 { 0 };
inline constexpr uint32_t LWM_PIN1 { 1 };
inline constexpr uint32_t LWM_PIN2 { 2 };
inline constexpr uint32_t LWM_PIN3 { 3 };

extern TIM_HandleTypeDef tim_lwm_h;
int tim_lwm_cfg();
#define TIM_LWM TIM2
#define TIM_LWM_EN  __GPIOA_CLK_ENABLE(); __TIM2_CLK_ENABLE();
#define TIM_LWM_DIS __TIM2_CLK_DISABLE();
#define TIM_LWM_GPIO_PIN_0 GPIO_PIN_0
#define TIM_LWM_GPIO_PIN_1 GPIO_PIN_1
#define TIM_LWM_GPIO_PIN_2 GPIO_PIN_2
#define TIM_LWM_GPIO_PIN_3 GPIO_PIN_3
#define TIM_LWM_GPIO_PINS ( TIM_LWM_GPIO_PIN_0 | TIM_LWM_GPIO_PIN_1 | TIM_LWM_GPIO_PIN_2 |  TIM_LWM_GPIO_PIN_3 )
#define TIM_LWM_GPIO_AF GPIO_AF1_TIM2
inline constexpr uint32_t tim_lwm_psc_freq   {  2000000 }; // 2 MHz
inline constexpr uint32_t tim_lwm_freq       {       50 }; // 50 Hz
inline constexpr uint32_t tim_lwm_t_us       { 1000000 / tim_lwm_freq };
int tim_lwm_cfg();
void tim_lwm_start();
void tim_lwm_stop();

inline auto& ADC1_GPIO { GpioA };
#define ADC1_NCH   3
#define ADC1_PIN0  GPIO_PIN_4
#define ADC1_PIN1  GPIO_PIN_5
#define ADC1_PIN2  GPIO_PIN_6
#define ADC1_PINS  ( ADC1_PIN0 | ADC1_PIN1 | ADC1_PIN2 )
#define ADC_CLK_EN __HAL_RCC_ADC1_CLK_ENABLE();  __HAL_RCC_GPIOA_CLK_ENABLE();
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
void MX_DMA_Init(void);
int  MX_ADC1_Init(void);
void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle );
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle);

class Mover;
class Sensor;

// ------------------------------- Coords -----------------------------------

using PartFun = float (*)(float); // [0..1] -> [0..1]
struct PartFunInfo {
  PartFun f;
  float kv;  // max speed koeff, <=1
};
inline float pafun_lim( float x ) { return std::clamp( x, 0.0f, 1.0f ); };
float pafun_one( float x );
float pafun_poly2_ss( float x ); // ss - slow start, se - low end, sb - slow both
float pafun_poly2_se( float x );
float pafun_poly3_sb( float x );
float pafun_trig_ss(  float x );
float pafun_trig_se(  float x );
float pafun_trig_sb(  float x );

extern const PartFunInfo part_fun_info[];

struct CoordInfo {
  float q_min, q_max;
  float vt_max;
  Sensor *sens;
  unsigned sens_ch;
  Mover *mo;
  float q_cur; // TODO: separate, other - const
};

constexpr size_t coords_n { 4 };
extern CoordInfo coords[coords_n];

struct MovePartCoord {
  float    q_e; // end point
  unsigned tp; // type of move per coord: enum or bifield?
  void init() { q_e = 0; tp = 0; };
};
OutStream& operator<<( OutStream &os, const MovePartCoord &rhs );

struct MovePart {
  static const constexpr unsigned n_max { 4 }; // <-> n_movers ??
  static const constexpr float kv_min { 0.01f };
  static const constexpr float kv_max { 2.00f };
  static const constexpr float kv_def { 0.50f };
  MovePartCoord mpc[n_max];
  float k_v; // velocity coeff
  void init() { for( auto &mp : mpc ) { mp.init(); }; k_v = kv_def; }
  void from_coords( std::span<const CoordInfo> coos, unsigned tp = 3 );
};
OutStream& operator<<( OutStream &os, const MovePart &rhs );


extern const MovePart mp_seq0[];
extern MovePart mp_stored;
extern MovePart mp_last;
inline const size_t mp_seq1_n { 20 };
extern       MovePart mp_seq1[mp_seq1_n];
extern size_t mp_seq1_sz;

bool is_overflow_seq();
int cmd2MovePart( int argc, const char * const * argv, int start_idx, MovePart &mp );

// ------------------------------------- Movers ----------------------------------------------

class Mover {
  public:
   enum Flags { noFlags = 0, offAfter = 1 };
   explicit Mover( float k_a_, float k_b_, float *fb_ = nullptr ) : fb( fb_ ), k_a( k_a_ ), k_b( k_b_ ) {};
   int move( float q, uint32_t t_cur );
   virtual int move_do( float q, uint32_t t_cur ) = 0;
   virtual int stop() = 0;
   virtual int init() = 0;
   virtual int pre_run( float q_e, unsigned tp, uint32_t nn )  { return 1; };
   virtual int post_run() { return 1; };
   virtual uint32_t getRaw() const { return 0; };
   virtual void setRaw( uint32_t rv ) {};
   virtual void setCtrlVal( uint32_t cv ) {}; // t_on for Servo
   virtual uint32_t getCtrlVal() const { return 0; };
   void set_t_old( uint32_t t_old_ ) { t_old = t_old_; }
   float get_q_last() const { return q_last; }
   void setFlags( Flags fl ) { flags = fl; };
  protected:
   float *fb; // feedback, not exsist = nullptr
   uint32_t t_old { 0 };
   float q_last  { 0 };
   float k_a, k_b;
   Flags flags { noFlags };
};

class MoverServoBase : public Mover {
  public:
    // TODO: TIM/ch ot tim_ctrl/ch
   MoverServoBase( float k_a_, float k_b_, __IO uint32_t &ccr_, __IO uint32_t &arr_, float *fb_ = nullptr )
     : Mover( k_a_, k_b_, fb_ ), ccr( ccr_ ), arr( arr_ ) {};
   virtual int move_do( float q, uint32_t t_cur ) override;
   virtual int stop() override { ccr = 0; return 1; };
   virtual int init() override { return 1; };
   virtual int post_run() override { if( flags & Flags::offAfter ) { stop(); }; return 1; };
   virtual uint32_t getRaw() const override { return ccr; };
   virtual void setRaw( uint32_t rv ) override { ccr = rv; };
   virtual void setCtrlVal( uint32_t cv ) override { t_on_last = cv; ccr = t_on2ccr( cv ); };
   virtual uint32_t getCtrlVal() const override { return t_on_last; };
   void set_lwm_times( uint32_t t_min, uint32_t t_max ) {
     t_on_min = t_min; t_on_max = t_max;
     t_on_cen = ( t_on_max_def + t_on_min_def ) / 2;
     t_on_dlt =   t_on_max_def - t_on_min_def;
   }
   uint32_t t_on2ccr( uint32_t t_on ) { return arr * t_on / tim_lwm_t_us; } // t_on in us
  protected:
   __IO uint32_t &ccr;
   __IO uint32_t &arr;
   uint32_t t_on_last { 0 };
   uint32_t t_on_min { t_on_min_def };
   uint32_t t_on_max { t_on_max_def };
   uint32_t t_on_cen { ( t_on_min_def + t_on_max_def)/2 };
   uint32_t t_on_dlt {   t_on_max_def - t_on_min_def };
   static const uint32_t t_on_min_def {  500 }; // min pulse width in us
   static const uint32_t t_on_max_def { 2500 }; // max pulse width in us
};


class MoverServo : public MoverServoBase {
  public:
   MoverServo( float k_a_, float k_b_, __IO uint32_t &ccr_, __IO uint32_t &arr_, float *fb_ = nullptr  )
     : MoverServoBase( k_a_, k_b_, ccr_, arr_, fb_ ) {};
   virtual int move_do( float q, uint32_t t_cur ) override;
  protected:
};


class MoverServoCont : public MoverServoBase {
  public:
   MoverServoCont( float k_a_, float k_b_, __IO uint32_t &ccr_, __IO uint32_t &arr_, float *fb_ = nullptr  )
     : MoverServoBase( k_a_, k_b_, ccr_, arr_, fb_ ) {};
   virtual int move_do( float q, uint32_t t_cur ) override;
   virtual int pre_run( float q_e, unsigned tp, uint32_t nn ) override;
   virtual int post_run() override;
  protected:
};

extern MoverServoCont mover_base;
extern MoverServo     mover_p1;
extern MoverServo     mover_p2;
extern MoverServo     mover_grip;
constexpr size_t movers_n { 4 };
extern std::array<Mover*,movers_n> movers;

// ------------------------------- Sensors -----------------------------------

class Sensor {
  public:
   explicit Sensor( unsigned n_ch_ ) : n_ch( n_ch_ ) {};
   virtual ~Sensor() = default;
   unsigned getNch() const { return n_ch; }
   virtual int measure( int nx ) = 0;
   virtual int init() = 0;
   virtual int32_t getInt( unsigned ch ) = 0;
   virtual float get( unsigned ch ) = 0;
   virtual void setCalibr( unsigned ch, const float *v ) = 0;
  protected:
   unsigned n_ch;
};

class SensorFakeMover : public Sensor {
  public:
   explicit SensorFakeMover( Mover &mo_  ) : Sensor( 1 ), mo( mo_ ) {};
   virtual ~SensorFakeMover() = default;
   virtual int measure( int nx ) override { x = mo.get_q_last(); return 1; }
   virtual int init() override { return 1; }
   virtual int32_t getInt( unsigned ch ) override { return ( uint32_t ) x * 10000000; }
   virtual float get( unsigned ch ) override { return x; };
   virtual void setCalibr( unsigned ch, const float *v ) override {};
  protected:
   Mover &mo;
   float x {0};
};

class SensorAdc : public Sensor {
  public:
   explicit SensorAdc( unsigned n_ch_ ) : Sensor( std::min( n_ch_, max_n_ch ) ) {};
   virtual ~SensorAdc() = default;
   virtual int measure( int nx ) override;
   virtual int init() override;
   virtual int32_t getInt( unsigned ch ) override { return ( ch < n_ch ) ? adc_data[ch] : 0; }
   virtual float get( unsigned ch ) override { return getInt(ch) * k_a[ch] + k_b[ch]; }
   virtual void setCalibr( unsigned ch, const float *v ) override { k_a[ch] = v[0]; k_b[ch] = v[1]; }
  protected:
   static const unsigned max_n_ch { 4 };
   int32_t  adc_data[max_n_ch];  // collected and divided data (by adc_measure)
   uint16_t adc_buf[max_n_ch];   // buffer for DMA
   float k_a[max_n_ch] {  7.0266e-02f,   -6.7271e-02f, 1.0f, 1.0f };
   float k_b[max_n_ch] {       -43.9f,           4.6f, 0.0f, 0.0f };
};

class SensorAS5600 : public Sensor {
  public:
   SensorAS5600( AS5600 &dev_) : Sensor( 1 ), dev( dev_ ) {};
   virtual ~SensorAS5600() = default;
   virtual int measure( int nx ) override;
   virtual int init() override;
   virtual int32_t getInt( unsigned /* ch */ ) override { return iv; }
   virtual float get( unsigned /* ch */ ) override { return v; };
   void set_zero_val( int zv ) { zero_val = zv; } // TODO: + calibr
   virtual void setCalibr( unsigned ch, const float *v ) override { k_a = v[0]; };
  protected:
   AS5600 &dev;
   float v;
   int zero_val { 0 };
   int32_t iv { 0 };
   float k_a { 180.0f / 2048 };
};

extern SensorAS5600 sens_enc;
extern SensorAdc sens_adc;
extern SensorFakeMover sens_grip;

int process_movepart( const MovePart &mp, float kkv = 1.0f  );
int run_moveparts( std::span<const MovePart> mps, float kkv = 1.0f, int i_start = 0, int i_end = 10000 );
int measure_store_coords( int nm );
void out_coords( bool nl );
void out_coords_int( bool nl );


#endif

