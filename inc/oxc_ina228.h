#ifndef _OXC_INA228_H
#define _OXC_INA228_H

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

     // reg_mask_en         = 0x06,
     // reg_alert_lim       = 0x07,
     // --------- cfg bits -----------
     cfg_default         = 0x0000,
     cfg_rst             = 0x8000,
     cfg_rstacc          = 0x4000,
     cfg_convdelay_mask  = 0x3FC0, // 8 bit,  1 bit = 2ms (initial ADC delay)
     cfg_tempcomp        = 0x0020,
     cfg_adcrange        = 0x0010, // 1 - 40.96 mV, 0 = 163.84 mV

     cfg_mode_down       = 0x0000,
     cfg_mode_cont       = 0x0004,
     cfg_mode_shunt_only = 0x0001,
     cfg_mode_bus_only   = 0x0002,
     cfg_mode_shunt_bus  = 0x0003,
     cfg_mode_def        = cfg_mode_cont | cfg_mode_shunt_bus,
     cfg_shunt_t_0140    = 0x0000, // in us
     cfg_shunt_t_0204    = 0x0008,
     cfg_shunt_t_0332    = 0x0010,
     cfg_shunt_t_0588    = 0x0018,
     cfg_shunt_t_1100    = 0x0020, // def
     cfg_shunt_t_2116    = 0x0028,
     cfg_shunt_t_4156    = 0x0030,
     cfg_shunt_t_8244    = 0x0038,
     cfg_bus_t_0140      = 0x0000, // in us
     cfg_bus_t_0204      = 0x0040,
     cfg_bus_t_0332      = 0x0080,
     cfg_bus_t_0588      = 0x00C0,
     cfg_bus_t_1100      = 0x0100, //def
     cfg_bus_t_2116      = 0x0140,
     cfg_bus_t_4156      = 0x0180,
     cfg_bus_t_8244      = 0x01C0,
     cfg_avg_1           = 0x0000,
     cfg_avg_4           = ( 0x0001 << 9 ),
     cfg_avg_16          = ( 0x0002 << 9 ),
     cfg_avg_64          = ( 0x0003 << 9 ),
     cfg_avg_128         = ( 0x0004 << 9 ),
     cfg_avg_256         = ( 0x0005 << 9 ),
     cfg_avg_512         = ( 0x0006 << 9 ),
     cfg_avg_1024        = ( 0x0007 << 9 ),
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
   bool setCfg(    uint16_t v )   { return writeReg( reg_cfg,     v ); };
   bool setAdcCfg( uint16_t v )   { return writeReg( reg_adccfg,  v ); };
   bool setCalibr( uint16_t v )   { return writeReg( reg_shunt_cal, v ); };
   uint32_t get_R_sh_uOhm() const { return R_sh_uOhm; }
   uint32_t get_I_lsb_mA() const  { return I_lsb_mA; }
   void set_calibr_val( uint32_t R_uOhm, uint32_t I_mA ) { R_sh_uOhm = R_uOhm; I_lsb_mA = I_mA; }
   bool calibrate()  { return setCalibr( 5120000 / ( I_lsb_mA * R_sh_uOhm ) ); }
   uint16_t getCfg() { return readReg( reg_cfg ); }
   uint16_t getAdcCfg() { return readReg( reg_adccfg ); }
   int32_t read24cvt( uint8_t reg );
   int32_t getVsh_raw()  { return last_Vsh  = read24cvt( reg_shunt_v ); }
   int32_t getVbus_raw() { return last_Vbus = read24cvt( reg_bus_v   ); };
   int32_t get_last_Vsh()  const { return last_Vsh; }
   int32_t get_last_Vbus() const { return last_Vbus; }
   int32_t getVbus_uV () { return getVbus_raw() * lsb_V_bus_uv; }
   int32_t getP()     { return read24cvt( reg_P ); }
   int32_t getI_raw() { return read24cvt( reg_I ); }
   int32_t Vsh2I_nA( int16_t v_raw ) const { return (int32_t)( (long long) v_raw * 1000 * lsb_V_sh_nv / R_sh_uOhm); }
   int32_t getI_nA() { return Vsh2I_nA( getVsh_raw() ); }
   int32_t getI_mA_reg() { return getI_raw() * I_lsb_mA; }
   uint16_t readReg( uint8_t reg ) { return recv_reg1_16bit_rev( reg, 0 ); };
   bool writeReg( uint8_t reg, uint16_t val ) { return send_reg1_16bit_rev( reg, val ) == 2; };
  protected:
   uint32_t R_sh_uOhm = 1500;
   uint32_t I_lsb_mA = 1;
   int32_t  last_Vsh  = 0;
   int32_t  last_Vbus = 0;
};

#endif
