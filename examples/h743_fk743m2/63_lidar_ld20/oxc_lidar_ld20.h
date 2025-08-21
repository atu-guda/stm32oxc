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
  uint16_t  alp_en;
  uint16_t  ts; //* timestamp
  uint8_t   crc;
  //
  bool is_good() const;
  uint8_t calc_crc() const;
  const uint8_t* cdata() const { return std::bit_cast<const uint8_t*>(this); };
  uint8_t*        data()       { return std::bit_cast<      uint8_t*>(this); };
  static const uint8_t crc_tab[256];

} __packed;

static_assert( sizeof(Lidar_LD20_Data) == Lidar_LD20_Data::pkgSize, "Bad Lidar_LD20_Data size, must be 47" );

class Lidar_LD20_Handler {
  public:
   Lidar_LD20_Handler() = default;
   void reset() { n = 0; nf = 0; pd0 = &d0; pd1 = &d1; };
   void add_byte( uint8_t v ); // beware: can be called from IRQ
   void add_bytes( const uint8_t *va, unsigned nb ) { for( unsigned i=0; i<nb; ++i ) add_byte( va[i] ); }
   const Lidar_LD20_Data* getData() const { return pd1; }
   unsigned get_nf() const { return nf; }
  private:
   Lidar_LD20_Data d0;
   Lidar_LD20_Data d1;
   Lidar_LD20_Data *pd0 { &d0 }; //* here to recieve
   Lidar_LD20_Data *pd1 { &d1 }; //* from here get
   int n { 0 };                  //* pos in receive + state
   unsigned nf { 0 };            //* number of frames, may be uint64_t?
};

#endif

