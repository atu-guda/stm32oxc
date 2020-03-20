#include <iostream>
#include <iomanip>
// #include <format>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <regex>
#include <cstring>
#include <getopt.h>

using namespace std;

int debug = 0;

struct PrmPtr {
  double *dv;
  int    *iv;
};
using PrmMap =  map<string,PrmPtr>;

void show_help( const char *prm_name );
string trim( const string &s );
bool parse_prm_line( const string &s, PrmMap &m );
void out_params( PrmMap &m );

int main( int argc, char **argv )
{
  int op;
  string ofile = "/dev/stdout"s;

  while( (op = getopt( argc, argv, "hdo:" ) ) != -1 ) {
    switch ( op ) {
      case 'h': show_help( argv[0] );  return 0;
      case 'd': debug++;               break;
      case 'o': ofile = optarg; break;
      default: cerr << "Unknown or bad option '"<< (char)optopt << "'" << endl;
               show_help( argv[0] );
               return 1;
    };
  };

  if( optind != argc-1 ) {
    cerr << "Error in parameters: need input filename" << endl;
    show_help( argv[0] );
    return 1;
  };

  string ifile = argv[optind];
  if( ifile == "-" ) {
    ifile = "/dev/stdin";
  }


  // parameters and interface to its
  const int max_ch = 32;
  int st = 0, n_row = 0, bpv = 0, sign = 0, n = 0, n_col = 0;
  int max_val = 0, v_ref_uV = 0, d_t_i = 0;
  unsigned signmask = 0;
  double d_t = 0;
  vector<double> k_n ( max_ch, 1.0 ); // not { } !!!!!

  PrmMap prms = {
    { "n_col"s    , PrmPtr{   nullptr, &n_col    } },
    { "n_row"s    , PrmPtr{   nullptr, &n_row    } },
    { "st"s       , PrmPtr{   nullptr, &st       } },
    { "bpv"s      , PrmPtr{   nullptr, &bpv      } },
    { "sign"s     , PrmPtr{   nullptr, &sign     } },
    { "n"s        , PrmPtr{   nullptr, &n        } },
    { "max_val"s  , PrmPtr{   nullptr, &max_val  } },
    { "v_ref_uV"s , PrmPtr{   nullptr, &v_ref_uV } },
    { "d_t_i"s    , PrmPtr{   nullptr, &d_t_i    } },
    { "d_t"s      , PrmPtr{      &d_t, nullptr   } },
    { "k_0"s      , PrmPtr{ &(k_n[0]), nullptr   } },
    { "k_1"s      , PrmPtr{ &(k_n[1]), nullptr   } },
    { "k_2"s      , PrmPtr{ &(k_n[2]), nullptr   } },
    { "k_3"s      , PrmPtr{ &(k_n[3]), nullptr   } },
    { "k_4"s      , PrmPtr{ &(k_n[4]), nullptr   } },
    { "k_5"s      , PrmPtr{ &(k_n[5]), nullptr   } },
    { "k_6"s      , PrmPtr{ &(k_n[6]), nullptr   } },
    { "k_7"s      , PrmPtr{ &(k_n[7]), nullptr   } },
    { "k_8"s      , PrmPtr{ &(k_n[8]), nullptr   } },
  };


  ifstream inf { ifile };
  if( !inf.is_open() ) {
    cerr << "Fail to open input file \"" << ifile << "\": "<< strerror(errno) << endl;
    return 2;
  }

  bool in_header = true;
  int c_row = st, line = 0; // line starts from 1: like editors
  vector<int> hv( max_ch, 0 );

  for( string s; getline( inf, s ); /* NOP */ ) {
    ++line;
    string s1 = trim( s );
    if( s1.empty() ) {
      continue;
    }

    if( s1[0] == '#' ) {
      cout << s1 << endl;
      if( s1[1] == '@' && ! in_header ) {
        cerr << "# warning: parameter after header in line " << line << endl;
        continue;
      }
      parse_prm_line( s1, prms );
      continue;
    }

    istringstream strs { s1 };
    int n_conv = 0;
    for( ;  strs && n_conv < (int)hv.size() ;  ) {
      int v = -1;
      strs >> hex >> v;
      if( strs ) {
        hv[n_conv++] = v;
      }
    };

    if( n_conv != n_col ) {
      cerr << "# warn: n_conv= " << n_conv << " != n_col = " << n_col << " in line " << line << endl;
      continue;
    }

    if( in_header ) { // header end detected
      in_header = false;
      out_params( prms );
      if( bpv < 4 || bpv > 31 || n_col < 1 || n < 1 || d_t <=0 ) {
        cerr << "# error: bad parameters detected!" << endl;
        return 4;
      }
      if( sign ) {
        signmask = ~0u << ( bpv - 1 );
        // cerr << "#- signmask= " << hex << signmask << endl;
      }
      c_row = st;
    }

    double c_t = d_t * c_row;

    cout << setw(15)  << setprecision(7) << c_t ;
    for( int i=0; i < n_col; ++i ) {
      int v = hv[i];
      if( signmask & hv[i] ) {
        v |= signmask;
      }
      double fv = v_ref_uV * 1e-6 * k_n[i] * v / max_val;
      cout << ' ' << setw(15) << setprecision(8)  <<  fv;
    }
    cout << endl;
    ++c_row;


  }

  return 0;
}


void show_help( const char *pname )
{
  cerr << "Usage: " << pname << " [opts] infile\n opts:\n";
  cerr << "  -h = help\n";
  cerr << "  -d = debug++\n";
  cerr << "  -o filename = outout file name\n";
}

bool parse_prm_line( const string &s, PrmMap &m )
{
  static regex data_cmt( "^#@\\s+([a-zA-Z0-9_]+)=\\s+(\\S+)" );
  smatch mr;
  if( regex_search( s, mr, data_cmt ) ) {
    string nm = mr[1];
    string val= mr[2];
    if( debug > 1 ) {
      cerr << "#**** " << nm << " = \"" << val << '"' << endl;
    }
    auto p = m.find( nm );
    if( p != m.end() ) {
      auto &prm = p->second;
      if( debug > 0 ) {
        cerr << "#+++ " << p->first << ' ' << prm.dv << endl;
      }
      if( prm.dv != nullptr ) {
        *(prm.dv) = stod( val );
      } else if( prm.iv != nullptr ) {
        *(prm.iv) = stoi( val, nullptr, 0 );
      } else {
        cerr << "# warn: ignored \"" << nm << '"' << endl;
      }
    } else {
      cerr << "# warn: not found parameter \"" << nm << '"' << endl;
      return true;
    }
  }
  return false;
}

void out_params( PrmMap &m )
{
  for( const auto& [ nm, p ] : m ) {
    cout << "# " << nm << "= ";

    if( p.dv != nullptr ) {
      cout << *(p.dv);
    } else if( p.iv != nullptr ) {
      cout << *(p.iv);
    } else {
      cout << '?';
    }

    cout << endl;
  }
}


string trim( const string &s )
{
  auto i = s.begin();
  for( ; i != s.end() && isspace( *i ) ; ++i ) {}

  auto ri = s.rbegin();
  for( ; ri.base() != i && isspace( *ri ); ++ri ) {}

  return string( i, ri.base() );
}
