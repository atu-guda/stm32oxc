#include <cstring>
#include <cstdlib>
#include <cerrno>

#include <oxc_gpio.h>
#include <oxc_usartio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_i2c.h>
#include <oxc_debug_i2c.h>
#include <oxc_common1.h>
#include <oxc_smallrl.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <lsm303dlhc.h>

using namespace std;
using namespace SMLRL;

// PinsOut p1 { GPIOE, 8, 8 };
BOARD_DEFINE_LEDS;



const int def_stksz = 1 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_print, smallrl_exec );

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


extern "C" {
  void task_main( void *prm UNUSED_ARG );
}

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0x19 );
void MX_I2C1_Init( I2C_HandleTypeDef &i2c );

UART_HandleTypeDef uah;
UsartIO usartio( &uah, USART2 );
int init_uart( UART_HandleTypeDef *uahp, int baud = 115200 );

STD_USART2_SEND_TASK( usartio );
// STD_USART2_RECV_TASK( usartio );
STD_USART2_IRQ( usartio );

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
  HAL_Init();

  SystemClock_Config();
  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );
  if( !init_uart( &uah ) ) {
    die4led( 0x08 );
  }
  leds.write( 0x0A );  delay_bad_ms( 200 );

  MX_I2C1_Init( i2ch );
  i2c_dbg = &i2cd;


  leds.write( 0x00 );

  UVAR('t') = 1000;
  UVAR('n') = 10;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart2_send, "send", 1*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  vTaskStartScheduler();
  die4led( 0xFF );



  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  uint32_t nl = 0;

  delay_ms( 50 );

  usartio.itEnable( UART_IT_RXNE );
  usartio.setOnSigInt( sigint );
  devio_fds[0] = &usartio; // stdin
  devio_fds[1] = &usartio; // stdout
  devio_fds[2] = &usartio; // stderr

  delay_ms( 50 );
  pr( "*=*** Main loop: ****** " NL );
  delay_ms( 20 );

  srl.setSigFun( smallrl_sigint );
  srl.set_ps1( "\033[32m#\033[0m ", 2 );
  srl.re_ps();
  srl.set_print_cmd( true );


  idle_flag = 1;
  while(1) {
    ++nl;
    if( idle_flag == 0 ) {
      pr_sd( ".. main idle  ", nl );
      srl.redraw();
    }
    idle_flag = 0;
    delay_ms( 60000 );
    // delay_ms( 1 );

  }
  vTaskDelete(NULL);
}

// ------------------------------------------------------------------------
// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int16_t buf[3];
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int t = UVAR('t');

  leds.set( BIT0 );
  delay_bad_mcs( 10 );

  break_flag = 0;
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

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;
  idle_flag = 1;

  pr( NL "test0 end." NL );
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

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;
  idle_flag = 1;

  pr( NL "test1 end." NL );
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
  // GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable DRDY clock */
  // ACCELERO_DRDY_GPIO_CLK_ENABLE();

  /* Enable INT1 & INT2 GPIO clock */
  // ACCELERO_INT_GPIO_CLK_ENABLE(); // = __GPIOE_CLK_ENABLE()

  /* Mems DRDY pin configuration */
  // GPIO_InitStructure.Pin = ACCELERO_DRDY_PIN;
  // GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  // GPIO_InitStructure.Pull  = GPIO_NOPULL;
  // GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
  // HAL_GPIO_Init(ACCELERO_DRDY_GPIO_PORT, &GPIO_InitStructure);

  /* Enable and set Button EXTI Interrupt to the lowest priority */
  // HAL_NVIC_SetPriority( ACCELERO_DRDY_EXTI_IRQn, 0x00, 0x00 );
  // HAL_NVIC_EnableIRQ( ACCELERO_DRDY_EXTI_IRQn );
  //
  // /* Configure GPIO PINs to detect Interrupts */
  // GPIO_InitStructure.Pin = ACCELERO_INT1_PIN | ACCELERO_INT2_PIN;
  // GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  // GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
  // GPIO_InitStructure.Pull  = GPIO_NOPULL;
  // HAL_GPIO_Init( ACCELERO_INT_GPIO_PORT, &GPIO_InitStructure );

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
  // GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
  // GPIO_InitStructure.Pull  = GPIO_NOPULL;
  // HAL_GPIO_Init( ACCELERO_INT_GPIO_PORT, &GPIO_InitStructure );
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

  if( status != HAL_OK )  {
    errno = 101;
    return 0;
    // I2Cx_Error();
  }
  return v;
}

//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

