#ifndef _OXC_INA228_H
#define _OXC_INA228_H

#include <utility>

// INA228 I2C 20-bit shunt + voltage (calc: I and P) sensor
// header-only

#include <oxc_i2c.h>

// inner regs: 1-byte addr, 2 byte data

class INA228 : public I2CClient {
  public:
   enum {
     def_addr            = 0x40, // 16 bit
     reg_cfg             = 0x00,
     reg_adccfg          = 0x01,
     reg_shunt_cal       = 0x02,
     reg_shunt_tempco    = 0x03,

     reg_shunt_v         = 0x04,  // 24 bit, 312.5 - 78.125 nV / LSB (ADCRANGE)
     reg_bus_v           = 0x05,  // 24 bit, 195.3125 uV / LSB, 4 low bits == 0
     reg_T               = 0x06,  // 16 bit
     reg_I               = 0x07,  // 24 bit
     reg_P               = 0x08,  // 24 bit
     reg_E               = 0x09,  // 40 bit
     reg_C               = 0x0A,  // 40 bit
     reg_diag            = 0x0B,  // 16 bit
     reg_SOVL            = 0x0B,  // 16 bit,  shunt overvoltage threshold
     reg_SUVL            = 0x0C,  // 16 bit,  shunt undervoltage threshold
     reg_BOVL            = 0x0D,  // 16 bit,  bus overvoltage threshold
     reg_BUVL            = 0x0E,  // 16 bit,  bus undervoltage threshold
     reg_TEMP_LIM        = 0x10,  // 16 bit,  temp over
     reg_PWR_LIM         = 0x11,  // 16 bit,  power over
     reg_id_manuf        = 0x3E,
     reg_id_dev          = 0x3F,

     // --------- cfg bits -----------
     cfg_rst             = 0x8000,
     cfg_rstacc          = 0x4000,
     cfg_convdelay_mask  = 0x3FC0, // 8 bit,  1 bit = 2ms (initial ADC delay)
     cfg_convdelay_shift = 6,      // shift 0xFF left to get initial conv delay
     cfg_tempcomp        = 0x0020,
     cfg_adcrange        = 0x0010, // 1 - 40.96 mV, 0 = 163.84 mV
     cfg_default         = 0x0000,
     cfg_def_scale1      = cfg_adcrange,

     acfg_mode_mask       = 0xF000,
     acfg_mode_down       = 0x0000,
     acfg_mode_trig_b     = 0x1000, // trigggered Vbus only
     acfg_mode_trig_s     = 0x2000, // trigggered shunt only
     acfg_mode_trig_bs    = 0x3000, // trigggered bus,shunt
     acfg_mode_trig_t     = 0x4000, // trigggered temp only
     acfg_mode_trig_bt    = 0x5000, // trigggered temp,vbus
     acfg_mode_trig_st    = 0x6000, // trigggered temp,shunt
     acfg_mode_trig_vst   = 0x7000, // trigggered all
     acfg_mode_cont_b     = 0x9000, // continuous Vbus only
     acfg_mode_cont_s     = 0xA000, // continuous shunt only
     acfg_mode_cont_bs    = 0xB000, // continuous bus,shunt - ok in w/o T
     acfg_mode_cont_t     = 0xC000, // continuous temp only
     acfg_mode_cont_bt    = 0xD000, // continuous temp,vbus
     acfg_mode_cont_st    = 0xE000, // continuous temp,shunt
     acfg_mode_cont_vst   = 0xF000, // continuous all - default

     acfg_ct_bus_0050    = (0u << 9), // in us
     acfg_ct_bus_0084    = (1u << 9),
     acfg_ct_bus_0150    = (2u << 9),
     acfg_ct_bus_0280    = (3u << 9),
     acfg_ct_bus_0540    = (4u << 9),
     acfg_ct_bus_1052    = (5u << 9), // def
     acfg_ct_bus_2074    = (6u << 9),
     acfg_ct_bus_4120    = (7u << 9),

     acfg_ct_sh_0050     = (0u << 6), // in us
     acfg_ct_sh_0084     = (1u << 6),
     acfg_ct_sh_0150     = (2u << 6),
     acfg_ct_sh_0280     = (3u << 6),
     acfg_ct_sh_0540     = (4u << 6),
     acfg_ct_sh_1052     = (5u << 6), // def
     acfg_ct_sh_2074     = (6u << 6),
     acfg_ct_sh_4120     = (7u << 6), // better

     acfg_ct_t_0050      = (0u << 3), // in us
     acfg_ct_t_0084      = (1u << 3),
     acfg_ct_t_0150      = (2u << 3),
     acfg_ct_t_0280      = (3u << 3),
     acfg_ct_t_0540      = (4u << 3),
     acfg_ct_t_1052      = (5u << 3), //def
     acfg_ct_t_2074      = (6u << 3),
     acfg_ct_t_4120      = (7u << 3),

     acfg_avg_mask        = 0x0007,
     acfg_avg_1           = 0x0000, // def
     acfg_avg_4           = 0x0001,
     acfg_avg_16          = 0x0002,
     acfg_avg_64          = 0x0003,
     acfg_avg_128         = 0x0004,
     acfg_avg_256         = 0x0005,
     acfg_avg_512         = 0x0006,
     acfg_avg_1024        = 0x0007,
     acfg_mode_def        = acfg_mode_cont_vst | acfg_ct_bus_1052 | acfg_ct_sh_1052 | acfg_ct_t_1052,

     // --------- mask_en bits -----------
     mask_en_len         = 0x0001,
     mask_en_apol        = 0x0002,
     mask_en_ovf         = 0x0004,
     mask_en_cvrf        = 0x0008,
     mask_en_aff         = 0x0010,
     mask_en_cnvr        = 0x0400,
     mask_en_pol         = 0x0800,
     mask_en_bul         = 0x1000,
     mask_en_bol         = 0x2000,
     mask_en_sul         = 0x4000,
     mask_en_sol         = 0x8000,
     // --------- id values -----------
     id_manuf            = 0x5449, // "TI"
     id_dev              = 0x2281,
   };
   enum {
     lsb_V_sh_nv         = 2500,
     lsb_V_bus_uv        = 1250
   };

   INA228( DevI2C &a_dev, uint8_t d_addr = def_addr )
     : I2CClient( a_dev, d_addr ) {};
   uint32_t isBad(); // returns 0 if good and VID:PID if bad
   bool setCfg(    uint16_t v )   { return writeReg( reg_cfg,     v ); };
   bool setAdcCfg( uint16_t v )   { return writeReg( reg_adccfg,  v ); };
   bool setCalibr( uint16_t v )   { return writeReg( reg_shunt_cal, v ); };
   uint32_t get_R_sh_uOhm() const { return R_sh_uOhm; }
   uint32_t get_I_lsb_mA() const  { return I_lsb_mA; }
   void set_calibr_val( uint32_t R_uOhm, uint32_t I_mA ) { R_sh_uOhm = R_uOhm; I_lsb_mA = I_mA; }
   bool calibrate()  { return setCalibr( 5120000 / ( I_lsb_mA * R_sh_uOhm ) ); }
   uint16_t getCfg() { return readReg( reg_cfg ); }
   uint16_t getAdcCfg() { return readReg( reg_adccfg ); }
   uint16_t getDiag()   { return (last_diag = readReg( reg_diag )); }
   uint16_t getLastDiag() const  { return last_diag; }
   int32_t read24cvt( uint8_t reg );
   int32_t getVsh()  { return last_Vsh  = read24cvt( reg_shunt_v ); }
   int32_t getVbus() { return last_Vbus = read24cvt( reg_bus_v   ); }
   std::pair<int32_t,int32_t> getVV() { return { getVsh(), getVbus() }; }
   int32_t getI() { return last_I = read24cvt( reg_I ); }
   int16_t getT() { return last_T = readReg( reg_T ); }
   int32_t get_last_Vsh()  const { return last_Vsh; }
   int32_t get_last_Vbus() const { return last_Vbus; }
   int32_t get_last_I() const { return last_I; }
   int32_t get_last_T() const { return last_T; }
   int32_t getVbus_uV () { return getVbus() * lsb_V_bus_uv; }
   int32_t getP()     { return read24cvt( reg_P ); }
   int32_t Vsh2I_nA( int16_t v_raw ) const { return (int32_t)( (long long) v_raw * 1000 * lsb_V_sh_nv / R_sh_uOhm); }
   int32_t getI_nA() { return Vsh2I_nA( getVsh() ); }
   int32_t getI_mA_reg() { return getI() * I_lsb_mA; }
   uint16_t readReg( uint8_t reg ) { return recv_reg1_16bit_rev( reg, 0 ); };
   bool writeReg( uint8_t reg, uint16_t val ) { return send_reg1_16bit_rev( reg, val ) == 2; };
   int waitEOC( int max_wait = 10000 ); // returs: 0: ok, 1-overtime, 2-break
   constexpr static inline uint16_t calc_acfg( uint8_t md, uint8_t ct_b, uint8_t ct_s, uint8_t ct_t, uint8_t avg )
     { return md << 12 | ((ct_b&7)<<9) | ((ct_s&7)<<6) | ((ct_t&7)<<3) | (avg&7); };
  protected:
   uint32_t R_sh_uOhm = 1500;
   uint32_t I_lsb_mA = 1;
   int32_t  last_Vsh  { 0 };
   int32_t  last_Vbus { 0 };
   int32_t  last_I    { 0 };
   int32_t  last_T    { 0 };
   int16_t  last_diag { 0 };
};

#endif
