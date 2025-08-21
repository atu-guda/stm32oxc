#ifndef _OXC_LIDAR_LD20_H
#define _OXC_LIDAR_LD20_H


struct Lidar_LD20_Data
{
  struct  OneSample  {
    uint16_t l;
    uint8_t  v;

  } __packed;
  enum { headByte = 0x54, vlenByte = 0x2C, pkgSize = 47, n_samples = 12 };

  uint8_t   head;  // headByte
  uint8_t   vlen;  // vlenByte
  uint16_t  speed;
  uint16_t  alp_st;
  OneSample d[n_samples];
  uint16_t  elp_en;
  uint16_t  ts; //* timestamp
  uint8_t   crc;
  //
  bool is_good() const;
  uint8_t calc_crc() const;
  static const uint8_t crc_tab[256];

} __packed;

static_assert( sizeof(Lidar_LD20_Data) == Lidar_LD20_Data::pkgSize, "Bad Lidar_LD20_Data size, must be 47" );


#endif

