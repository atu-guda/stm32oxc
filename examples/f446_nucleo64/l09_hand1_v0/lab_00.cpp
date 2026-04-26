#include <cstdint>
#include <cmath>

#include "lab0_common.h"

// variant 0
static const TaskPart lab_task0[] {
  //  q_e      t     tp     flg
  {   0.0f,  4000,    3,     0 },
  {  90.0f,  4000,    3,     0 },
  { -90.0f,  4000,    3,     0 },
  {  10.0f,  2000,    3,     0 },
  { -10.0f,  5000,    3,     0 },
  {   0.0f,  1000,    3,     0 },
};

// variant 1
static const TaskPart lab_task1[] {
  //  q_e      t     tp     flg
  {   0.0f,  4000,    3,     0 },
  { -90.0f,  4000,    3,     0 },
  {  20.0f,  6000,    3,     0 },
  { -30.0f,  2000,    3,     0 },
  {  10.0f,  1000,    0,     0 },
  {   0.0f,  1000,    3,     0 },
};

// variant 2
static const TaskPart lab_task2[] {
  //  q_e      t     tp     flg
  {   0.0f,  4000,    3,     0 },
  {  10.0f,  5000,    3,     0 },
  { -90.0f,  4000,    3,     0 },
  {  50.0f,  3000,    3,     0 },
  { -20.0f,  2000,    0,     0 },
  {   0.0f,  1000,    3,     0 },
};

// variant 3
static const TaskPart lab_task3[] {
  //  q_e      t     tp     flg
  {   0.0f,  4000,    3,     0 },
  { -10.0f,  4000,    3,     0 },
  { -90.0f,  4000,    3,     0 },
  {  25.0f,  3000,    3,     0 },
  { -10.0f,  2000,    0,     0 },
  {   0.0f,  1000,    3,     0 },
};

int lab_init( int x )
{
  return 0;
}

int lab_step( uint32_t tc )
{
  if( tc < (uint32_t)t_lab_max ) {
    set_l0_v( 0.2 );
    return 0;
  }
  return 1;
}


