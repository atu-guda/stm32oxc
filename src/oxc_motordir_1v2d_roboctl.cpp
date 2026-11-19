#include <oxc_motordir_1v2d_roboctl.h>

using namespace oxc;


int32_t_er oxc::MotorDir_1V2D_RoboCtl::getVal(  size_t ch ) noexcept
{
  switch( ch ) {
    case ch_left_out  : return o_left;
    case ch_right_out : return o_right;
  }
  return std::unexpected( rcErr );
}

float_er   oxc::MotorDir_1V2D_RoboCtl::getValF( size_t ch ) noexcept
{
  switch( ch ) {
    case ch_v_in  : return vi;
    case ch_v_out : return vo;
  }
  return std::unexpected( rcErr );
}

ReturnCode oxc::MotorDir_1V2D_RoboCtl::doInit()    noexcept
{
  vi = vo = 0;
  o_left = o_right = 0;
  return rcOk;
}

ReturnCode oxc::MotorDir_1V2D_RoboCtl::doMeasure() noexcept
{
  return rcOk;
}

ReturnCode oxc::MotorDir_1V2D_RoboCtl::doThink()   noexcept
{
  vi = std::clamp( v_in.in(), -1.0f, 1.0f );
  auto idle = idle_in.in();
  auto brk  = break_in.in();

  vo = fabsf( vi );
  if( vo < zero_v ) {
    brk  |= break_v0 ? 1 : 0;
    idle |= break_v0 ? 0 : 1;
  }

  if( idle ) {
    do_idle();
    vo = 0; v_out.out( vo );
    return rcOk;
  }

  if( brk ) {
    do_break();
    vo = 0; v_out.out( vo );
    return rcOk;
  }

  if( vi < 0 ) {
    left_out.out(  1 );
    right_out.out( 0 );
  } else {
    left_out.out(  0 );
    right_out.out( 1 );
  }
  v_out.out( vo );

  return rcOk;
}

ReturnCode oxc::MotorDir_1V2D_RoboCtl::doCommit()  noexcept
{
  return rcOk;
}

void oxc::MotorDir_1V2D_RoboCtl::do_break() noexcept
{
  uint32_t ov = ( break_low ) ? 0 : 1;
  left_out.out( ov );
  right_out.out( ov );
}

void oxc::MotorDir_1V2D_RoboCtl::do_idle()  noexcept
{
  uint32_t ov = ( break_low ) ? 1 : 0;
  left_out.out( ov );
  right_out.out( ov );
}

