#include <cerrno>

#include <oxc_auto.h>
#include <oxc_main.h>

#include <lsm303dlhc.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_test1( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST1 { "test1", 'X', cmd_test1, " - I2C actions"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_TEST1,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0x19 );

// --------------------- peripherals defs --------------------------

// TODO: to board file
#define ACCELERO_I2C_ADDRESS             0x32

#define ACCELERO_DRDY_PIN                GPIO_PIN_2                  /* PE.02 */
#define ACCELERO_DRDY_GPIO_PORT          GPIOE                       /* GPIOE */
#define ACCELERO_DRDY_GPIO_CLK_ENABLE()  __GPIOE_CLK_ENABLE()
#define ACCELERO_DRDY_GPIO_CLK_DISABLE() __GPIOE_CLK_DISABLE()
#define ACCELERO_DRDY_EXTI_IRQn          EXTI2_TSC_IRQn              /*TAMP_STAMP_IRQn*/

#define ACCELERO_INT_GPIO_PORT           GPIOE                       /* GPIOE */
#define ACCELERO_INT_GPIO_CLK_ENABLE()   __GPIOE_CLK_ENABLE()
#define ACCELERO_INT_GPIO_CLK_DISABLE()  __GPIOE_CLK_DISABLE()
#define ACCELERO_INT1_PIN                GPIO_PIN_4                  /* PE.04 */
#define ACCELERO_INT1_EXTI_IRQn          EXTI4_IRQn
#define ACCELERO_INT2_PIN                GPIO_PIN_5                  /* PE.05 */
#define ACCELERO_INT2_EXTI_IRQn          EXTI9_5_IRQn

typedef enum
{
  ACCELERO_OK = 0,
  ACCELERO_ERROR = 1,
  ACCELERO_TIMEOUT = 2
}
ACCELERO_StatusTypeDef;
uint8_t   BSP_ACCELERO_Init();
void      BSP_ACCELERO_Reset();
void      BSP_ACCELERO_GetXYZ( int16_t *pDataXYZ );
ACCELERO_DrvTypeDef *AccelerometerDrv;
void      COMPASSACCELERO_IO_Init(void);
void      COMPASSACCELERO_IO_ITConfig(void);
void      COMPASSACCELERO_IO_Write(uint16_t DeviceAddr, uint8_t RegisterAddr, uint8_t Value);
uint8_t   COMPASSACCELERO_IO_Read(uint16_t DeviceAddr, uint8_t RegisterAddr);

// --------------------- peripherals defs end --------------------------

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;

  BOARD_POST_INIT_BLINK;

  BOARD_CREATE_STD_TASKS;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  default_main_loop();
  vTaskDelete(NULL);
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int16_t buf[3];
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int t = UVAR('t');

  leds.set( BIT0 );
  delay_bad_mcs( 10 );

  // LSM303DLHC_AccInit( 0 );
  BSP_ACCELERO_Init();

  for( int i=0; i<n && !break_flag; ++i ) {
    BSP_ACCELERO_GetXYZ( buf );
    pr( "[ " ); pr_d( buf[0] );
    pr( " ; " ); pr_d( buf[1] );
    pr( " ; " ); pr_d( buf[2] );
    pr( " ]" NL );
    delay_ms( t );
  }

  leds.reset( BIT0 );

  return 0;
}

int cmd_test1( int argc, const char * const * argv )
{
  uint8_t buf[4];
  buf[0] = buf[1] = buf[2] = buf[3] = 0;

  leds.set( BIT0 );
  delay_bad_mcs( 10 );

  int rc = i2cd.recv_reg1( 0x0F, buf, 1 );
  pr_sdx( rc );
  dump8( buf, 4 );

  leds.reset( BIT0 );
  return 0;
}
//  ----------------------------- devices ----------------

uint8_t BSP_ACCELERO_Init(void)
{
  uint8_t ret = ACCELERO_ERROR;
  uint16_t ctrl = 0x0000;
  ACCELERO_InitTypeDef accel_cfg;
  ACCELERO_FilterConfigTypeDef filter_cfg;

  if( Lsm303dlhcDrv.ReadID() == I_AM_LMS303DLHC ) {
    AccelerometerDrv = &Lsm303dlhcDrv;

    /* MEMS configuration ------------------------------------------------------*/
    accel_cfg.Power_Mode = LSM303DLHC_NORMAL_MODE;
    accel_cfg.AccOutput_DataRate = LSM303DLHC_ODR_50_HZ;
    accel_cfg.Axes_Enable= LSM303DLHC_AXES_ENABLE;
    accel_cfg.AccFull_Scale = LSM303DLHC_FULLSCALE_2G;
    // accel_cfg.AccFull_Scale = LSM303DLHC_FULLSCALE_8G;
    accel_cfg.BlockData_Update = LSM303DLHC_BlockUpdate_Continous;
    accel_cfg.Endianness = LSM303DLHC_BLE_LSB;
    accel_cfg.High_Resolution = LSM303DLHC_HR_ENABLE;

    /* Configure MEMS: data rate, power mode, full scale and axes */
    ctrl |= (accel_cfg.Power_Mode | accel_cfg.AccOutput_DataRate | \
        accel_cfg.Axes_Enable);

    ctrl |= ((accel_cfg.BlockData_Update | accel_cfg.Endianness | \
              accel_cfg.AccFull_Scale | accel_cfg.High_Resolution) << 8);

    /* Configure the accelerometer main parameters */
    AccelerometerDrv->Init(ctrl);

    /* Fill the accelerometer LPF structure */
    filter_cfg.HighPassFilter_Mode_Selection = LSM303DLHC_HPM_NORMAL_MODE;
    filter_cfg.HighPassFilter_CutOff_Frequency = LSM303DLHC_HPFCF_16;
    filter_cfg.HighPassFilter_AOI1 = LSM303DLHC_HPF_AOI1_DISABLE;
    filter_cfg.HighPassFilter_AOI2 = LSM303DLHC_HPF_AOI2_DISABLE;

    /* Configure MEMS: mode, cutoff frquency, Filter status, Click, AOI1 and AOI2 */
    ctrl = (uint8_t) (filter_cfg.HighPassFilter_Mode_Selection |\
        filter_cfg.HighPassFilter_CutOff_Frequency|\
        filter_cfg.HighPassFilter_AOI1|\
        filter_cfg.HighPassFilter_AOI2);

    /* Configure the accelerometer LPF main parameters */
    AccelerometerDrv->FilterConfig( ctrl );

    ret = ACCELERO_OK;
  } else  {
    ret = ACCELERO_ERROR;
  }

  return ret;
}

void BSP_ACCELERO_Reset(void)
{
  if( AccelerometerDrv->Reset != NULL )  {
    AccelerometerDrv->Reset();
  }
}

void BSP_ACCELERO_GetXYZ( int16_t *pDataXYZ )
{
  if( AccelerometerDrv->GetXYZ != NULL )  {
    AccelerometerDrv->GetXYZ( pDataXYZ );
  }
}

// void BSP_COMPAS_GetXYZ( int16_t *pDataXYZ )
// {
//   if( CompasDrv->GetXYZ != NULL )  {
//     CompasDrv->GetXYZ( pDataXYZ );
//   }
// }

void COMPASSACCELERO_IO_Init(void)
{
  /* Enable DRDY clock */
  // ACCELERO_DRDY_GPIO_CLK_ENABLE();

  /* Enable INT1 & INT2 GPIO clock */
  // ACCELERO_INT_GPIO_CLK_ENABLE(); // = __GPIOE_CLK_ENABLE()

  /* Mems DRDY pin configuration */
  // ACCELERO_DRDY_GPIO_PORT.cfgIn_N( ACCELERO_DRDY_PIN );

  /* Enable and set Button EXTI Interrupt to the lowest priority */
  // HAL_NVIC_SetPriority( ACCELERO_DRDY_EXTI_IRQn, 0x00, 0x00 );
  // HAL_NVIC_EnableIRQ( ACCELERO_DRDY_EXTI_IRQn );
  //
  // // Configure GPIO PINs to detect Interrupts
  // ACCELERO_INT_GPIO_PORT.cfgIn_N(  ACCELERO_INT1_PIN | ACCELERO_INT2_PIN );

  // I2Cx_Init();
}

void COMPASSACCELERO_IO_ITConfig(void)
{
  // GPIO_InitTypeDef GPIO_InitStructure;
  //
  // /* Enable INT1 & INT2 GPIO clock */
  // ACCELERO_INT_GPIO_CLK_ENABLE();
  //
  // /* Configure GPIO PINs to detect Interrupts */
  // GPIO_InitStructure.Pin = ACCELERO_INT1_PIN | ACCELERO_INT2_PIN;
  // GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
  // GPIO_InitStructure.Speed = GPIO_SPEED_MAX;
  // GPIO_InitStructure.Pull  = GPIO_NOPULL;
  // ACCELERO_INT_GPIO_PORT.cfgIn_N( ACCELERO_INT1_PIN | ACCELERO_INT2_PIN );
  // TODO: EXTI
  //
  // /* Enable and set Button EXTI Interrupt to the lowest priority */
  // HAL_NVIC_SetPriority(ACCELERO_INT1_EXTI_IRQn, 0x00, 0x00);
  // HAL_NVIC_EnableIRQ(ACCELERO_INT1_EXTI_IRQn);

}

void COMPASSACCELERO_IO_Write( uint16_t DeviceAddr, uint8_t RegisterAddr, uint8_t v )
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_I2C_Mem_Write( &i2ch, DeviceAddr, (uint16_t)RegisterAddr, I2C_MEMADD_SIZE_8BIT,
      &v, 1, 100 /*I2cxTimeout*/);

  pr( NL "W: dev: " ); pr_h( DeviceAddr ); pr( " reg: " ); pr_h( RegisterAddr );
  pr( " <- " ); pr_h( v ); pr( NL );

  if( status != HAL_OK )  {
    errno = 108;
    // I2Cx_Error();
  }
}

uint8_t COMPASSACCELERO_IO_Read(uint16_t DeviceAddr, uint8_t RegisterAddr)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t v = 0;

  status = HAL_I2C_Mem_Read( &i2ch, DeviceAddr, RegisterAddr, I2C_MEMADD_SIZE_8BIT,
      &v, 1, 100 /*I2cxTimeout*/ );

  pr( NL "R: dev: " ); pr_h( DeviceAddr ); pr( " reg: " ); pr_h( RegisterAddr );
  pr( " -> " ); pr_h( v ); pr( NL );

  if( status != HAL_OK )  {
    errno = 101;
    return 0;
    // I2Cx_Error();
  }
  return v;
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

