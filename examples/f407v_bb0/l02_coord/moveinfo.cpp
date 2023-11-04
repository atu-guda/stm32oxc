#include <ranges>
#include <algorithm>
#include <cmath>

#include "moveinfo.h"

using namespace std;

#define OUT std_out

// -------------------- MoveInfo ------------------------------

MoveInfo::MoveInfo( unsigned a_n_coo, Act_Pfun pfun  )
  : n_coo( a_n_coo ), step_pfun( pfun )
{
}

void MoveInfo::zero_arr()
{
  ranges::fill( k_x, 0 );
}

ReturnCode MoveInfo::prep_move_line( const xfloat *prm )
{
  zero_arr(); len = 0;

  pa = prm;
  for( unsigned i=0; i<n_coo; ++i ) {
    len += prm[i] * prm[i];
  }
  len = sqrtxf( len );

  OUT << "# debug: prep_move_line: len= " << len << " n_coo=" << n_coo;
  for( unsigned i=0; i<n_coo && i < max_n_koeffs; ++i ) {
    k_x[i] = prm[i];
    OUT << ' ' << k_x[i];
  }
  OUT << NL;

  return rcOk;
}

ReturnCode MoveInfo::prep_move_circ( const xfloat *prm )
{
  zero_arr();
  pa = prm;

  // copy from g_move_circle + const
  const xfloat &r_s   { prm[0]  };
  const xfloat &alp_s { prm[1]  };
  const xfloat &r_e   { prm[2]  };
  const xfloat &alp_e { prm[3]  };
  // const xfloat &cv    { prm[4]  };
  const xfloat &z_e   { prm[5]  };
  // const xfloat &e_e   { prm[6]  };
  // const xfloat &nt    { prm[7]  };
  // const xfloat &x_e   { prm[8]  };
  // const xfloat &y_e   { prm[9]  };
  // const xfloat &x_r   { prm[10] };
  // const xfloat &y_r   { prm[11] };
  // const xfloat &r_1   { prm[12] };

  xfloat l_appr { 0.5f * ( r_s + r_s ) * fabsxf( alp_e - alp_s ) };
  len = hypot( l_appr, r_e - r_s, z_e ); // e_e ?

  k_x[0] = alp_e - alp_s;
  k_x[1] = r_e   - r_s;
  k_x[2] = - ( r_s * cos( alp_s ) ); // initial point shift
  k_x[3] = - ( r_s * sin( alp_s ) );

  OUT << "# debug: prep_move_circ: len= " << len;
  for( unsigned i=0; i<6; ++i ) {
    OUT << ' ' << k_x[i];
  }
  OUT << NL;

  return rcOk;
}

ReturnCode MoveInfo::calc_step( xfloat a, xfloat *coo )
{
  if( !isGood() || coo == nullptr ) {
    return ReturnCode::rcErr;
  }
  return step_pfun( *this, a, coo );
}

// not a member - to allow external funcs
ReturnCode step_line_fun( MoveInfo &mi, xfloat a, xfloat *coo )
{
  for( unsigned i=0; i<mi.n_coo; ++i ) { // TODO: common?
    coo[i] =  a * mi.k_x[i];
  }

  return ReturnCode::rcOk;
}

ReturnCode step_circ_fun( MoveInfo &mi, xfloat a, xfloat *coo )
{
  // aliases
  const xfloat &r_s   { mi.pa[0]  };
  const xfloat &alp_s { mi.pa[1]  };
  const xfloat &z_e   { mi.pa[5]   };
  const xfloat &e_e   { mi.pa[6]   };

  const xfloat &k_alp { mi.k_x[0] };
  const xfloat &k_r   { mi.k_x[1] };
  const xfloat &x_0   { mi.k_x[2] };
  const xfloat &y_0   { mi.k_x[3] };

  const xfloat alp =  alp_s + a * k_alp;
  const xfloat r   =  r_s   + a * k_r;

  coo[0]  = x_0 + r * cos( alp );
  coo[1]  = y_0 + r * sin( alp );
  coo[2]  = a * z_e;
  coo[3]  = a * e_e;

  return ReturnCode::rcOk;
}
