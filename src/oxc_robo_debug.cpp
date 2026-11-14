#include <ranges>

#include <oxc_auto.h>
#include <oxc_floatfun.h>

#include <oxc_robo_base.h>

using namespace oxc;

// ------------------------ - local commands; ---------------------------------------
DCL_CMD_REG(      robo_wi,      '\0',     " i ch vi - write robo int"  );
DCL_CMD_REG(      robo_wf,      '\0',     " i ch vf - write robo float"  );
DCL_CMD_REG(      robo_ri,      '\0',     " i ch - read robo int"  );
DCL_CMD_REG(      robo_rf,      '\0',     " i ch - read robo float"  );
DCL_CMD_REG(      robo_meas,    '\0',     " i - measure robo"  );
DCL_CMD_REG(      robo_co,      '\0',     " i - commit robo"  );
DCL_CMD_REG(      robo_list,    '\0',     " - list robo objects"  );
DCL_CMD_REG(      robo_init,    '\0',     " - init robo objects"  );
DCL_CMD_REG(      out_cf,       '\0',     " i_och v - output to float ch"  );
DCL_CMD_REG(      out_ci,       '\0',     " i_och vi - output to int ch"  );
DCL_CMD_REG(      in_cf,        '\0',     " [idx] read float ch /all"  );
DCL_CMD_REG(      in_ci,        '\0',     " [idx] read int  ch /all"  );



CMD_FUNCTION( robo_wi )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, oxc::global_robo_caps.size()-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  int32_t vi  = arg2long_d(  3, argc, argv, 0 );
  auto no_commit = arg2long_d( 4, argc, argv, 0 );
  auto rc = global_robo_caps[idx]->setVal( ch, vi );
  std_out << "# idx= " << idx << " ch= " << ch
          << " rc= " << rc.code << ' ' << rc.data << NL;
  if( !no_commit ) {
    rc = global_robo_caps[idx]->commit();
    std_out << "# commit rc= " << rc.code << ' ' << rc.data << NL;
  }
  return 0;
}


CMD_FUNCTION( robo_wf )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, oxc::global_robo_caps.size()-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  float vf = arg2float_d( 3, argc, argv, 0 );
  auto no_commit = arg2long_d( 4, argc, argv, 0 );
  auto rc = global_robo_caps[idx]->setValF( ch, vf );
  std_out << "# idx= " << idx << " ch= " << ch
          << " rc= " << rc.code << ' ' << rc.data << NL;
  if( !no_commit ) {
    rc = global_robo_caps[idx]->commit();
    std_out << "# commit rc= " << rc.code << ' ' << rc.data << NL;
  }
  return 0;
}


CMD_FUNCTION( robo_ri )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, oxc::global_robo_caps.size()-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  auto r = global_robo_caps[idx]->getVal( ch );
  std_out << "# idx= " << idx << " ch= " << ch;
  if( r ) {
    std_out << " val= " << r.value() << NL;
  } else {
    std_out << " err " << r.error().code << NL;
  }
  return 0;
}

CMD_FUNCTION( robo_rf )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, oxc::global_robo_caps.size()-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  auto r = global_robo_caps[idx]->getValF( ch );
  std_out << "# idx= " << idx << " ch= " << ch;
  if( r ) {
    std_out << " val= " << r.value() << NL;
  } else {
    std_out << " err " << r.error().code << NL;
  }
  return 0;
}

CMD_FUNCTION( robo_meas )
{
  auto idx = arg2ulong_d( 1, argc, argv, 1000000, 0 ); // all by default
  auto rc = ( idx >= global_robo_objs.size() ) ? global_robo.measure_all() : global_robo_objs[idx]->measure();
  std_out << "# rc= " << rc.code << ' ' << rc.data << " idx= " << idx << NL;
  return 0;
}

CMD_FUNCTION( robo_co )
{
  auto idx = arg2ulong_d( 1, argc, argv, 1000000, 0 ); // all by default
  auto rc = ( idx >= global_robo_objs.size() ) ? global_robo.commit_all() : global_robo_objs[idx]->commit();
  std_out << "# rc= " << rc.code << ' ' << rc.data << " idx= " << idx << NL;
  return 0;
}

CMD_FUNCTION( robo_list )
{
  for( auto [i,pob] : std::views::enumerate( global_robo_objs ) ) {
    std_out << i << ' ' << pob->getId() << ' ' << pob->getStatus() << ' ' << pob->getDirty()
            << ' ' << pob->getFlags() << ' ' << HexInt(pob) << NL;
  }
  return 0;
}

CMD_FUNCTION( robo_init )
{
  auto rc = oxc::global_robo.init_all();
  std_out << " rc= [" << rc.code << ", " << rc.data << "]" NL;
  return 0;
}


CMD_FUNCTION( out_cf ) // F
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, global_channels_out_f.size()-1 );
  float v  = arg2float_d( 2, argc, argv, 0 );
  auto no_commit = arg2long_d( 3, argc, argv, 0 );
  global_channels_out_f[idx]->out( v );
  std_out << "# idx= " << idx << " v= " << v << NL;
  if( !no_commit ) {
    global_robo.commit_all();
  }
  return 0;
}

CMD_FUNCTION( out_ci )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, global_channels_out_i.size()-1 );
  int32_t v  = arg2long_d( 2, argc, argv, 0 );
  auto no_commit = arg2long_d( 3, argc, argv, 0 );
  global_channels_out_i[idx]->out( v );
  std_out << "# idx= " << idx << " v= " << v << NL;
  if( !no_commit ) {
    global_robo.commit_all();
  }
  return 0;
}


CMD_FUNCTION( in_cf )
{
  global_robo.measure_all();
  auto idx = arg2ulong_d( 1, argc, argv, 1000000, 1000000, oxc::global_channels_in_f.size()-1 );

  // single channel
  if( idx <= oxc::global_channels_in_f.size() ) {
    auto v = global_channels_in_f[idx]->in();
    std_out << idx << ' ' << v << NL;
    return 0;
  }

  // all channels
  for( auto [i,pich] : std::views::enumerate( oxc::global_channels_in_f ) ) {
    auto v = pich->in();
    std_out << i << ' ' << v << NL;
  }
  return 0;
}

CMD_FUNCTION( in_ci )
{
  global_robo.measure_all();
  auto idx = arg2ulong_d( 1, argc, argv, 1000000, 1000000, oxc::global_channels_in_i.size()-1 );

  // single channel
  if( idx <= oxc::global_channels_in_i.size() ) {
    auto v = oxc::global_channels_in_i[idx]->in();
    std_out << idx << ' ' << v << NL;
    return 0;
  }

  // all channels
  for( auto [i,pich] : std::views::enumerate( oxc::global_channels_in_i ) ) {
    auto v = pich->in();
    std_out << i << ' ' << v << NL;
  }
  return 0;
}


