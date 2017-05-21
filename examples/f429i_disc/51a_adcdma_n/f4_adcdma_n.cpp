// file
// base on file    stm32f4xx_hal_adc.c

#include <oxc_base.h>
#include <oxc_gpio.h>
#include <oxc_debug1.h>

extern  PinsOut ledsx; // debug


//
static void ADC_DMAConvCplt_n( DMA_HandleTypeDef *hdma );
static void ADC_DMAError_n( DMA_HandleTypeDef *hdma );
static void ADC_DMAHalfConvCplt_n( DMA_HandleTypeDef *hdma );


HAL_StatusTypeDef ADC_Start_DMA_n( ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length, uint32_t /* chunkLength */ )
{
  __IO uint32_t counter = 0U;
  ADC_Common_TypeDef *tmpADC_Common;

  __HAL_LOCK( hadc );

  // Enable the ADC peripheral and wait for stabilizetion
  if( ( hadc->Instance->CR2 & ADC_CR2_ADON ) != ADC_CR2_ADON ) {
    __HAL_ADC_ENABLE( hadc );

    counter = ( ADC_STAB_DELAY_US * ( SystemCoreClock / 1000000U ) );
    while( counter != 0U ) {
      counter--;
    }
  }

  // Start conversion if ADC is effectively enabled
  if( HAL_IS_BIT_SET( hadc->Instance->CR2, ADC_CR2_ADON ) ) {
    // Set ADC state
    // - Clear state bitfield related to regular group conversion results
    // - Set state bitfield related to regular group operation
    ADC_STATE_CLR_SET( hadc->State,
                      HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC | HAL_ADC_STATE_REG_OVR,
                      HAL_ADC_STATE_REG_BUSY );

    // injected group ignored,
    // Reset ADC all error code fields
    ADC_CLEAR_ERRORCODE( hadc );

    __HAL_UNLOCK( hadc );

    // Pointer to the common control register to which is belonging hadc
    // ( Depending on STM32F4 product, there may be up to 3 ADCs and 1 common control register )
    tmpADC_Common = ADC_COMMON_REGISTER( hadc );

    // Set the DMA transfer complete callback
    hadc->DMA_Handle->XferCpltCallback = ADC_DMAConvCplt_n;

    // Set the DMA half transfer complete callback
    hadc->DMA_Handle->XferHalfCpltCallback = ADC_DMAHalfConvCplt_n;

    // Set the DMA error callback
    hadc->DMA_Handle->XferErrorCallback = ADC_DMAError_n;


    // Manage ADC and DMA start: ADC overrun interruption, DMA start, ADC
    // start ( in case of SW start ):

    // Clear regular group conversion flag and overrun flag
    // ( To ensure of no unknown state from potential previous ADC operations )
    __HAL_ADC_CLEAR_FLAG( hadc, ADC_FLAG_EOC | ADC_FLAG_OVR );

    // Enable ADC overrun interrupt
    __HAL_ADC_ENABLE_IT( hadc, ADC_IT_OVR );

    // Enable ADC DMA mode
    hadc->Instance->CR2 |= ADC_CR2_DMA;

    // Start the DMA channel // atu: TODO: here!
    HAL_DMA_Start_IT( hadc->DMA_Handle, ( uint32_t )&hadc->Instance->DR, ( uint32_t )pData, Length );

    // Check if Multimode enabled
    if( HAL_IS_BIT_CLR( tmpADC_Common->CCR, ADC_CCR_MULTI ) ) {
      // if no external trigger present enable software conversion of regular channels
      if( ( hadc->Instance->CR2 & ADC_CR2_EXTEN ) == RESET ) {
        // Enable the selected ADC software conversion for regular group
        hadc->Instance->CR2 |= ( uint32_t )ADC_CR2_SWSTART;
      }
    } else {
      // if instance of handle correspond to ADC1 and  no external trigger present enable software conversion of regular channels
      if( ( hadc->Instance == ADC1 ) && ( ( hadc->Instance->CR2 & ADC_CR2_EXTEN ) == RESET ) ) {
        // Enable the selected ADC software conversion for regular group
          hadc->Instance->CR2 |= ( uint32_t )ADC_CR2_SWSTART;
      }
    }
  }

  return HAL_OK;
}


/**
  * @brief  DMA transfer complete callback.
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
static void ADC_DMAConvCplt_n( DMA_HandleTypeDef *hdma )
{
  log_add( "ADCC" NL );
  // Retrieve ADC handle corresponding to current DMA handle
  ADC_HandleTypeDef* hadc = ( ADC_HandleTypeDef* )( ( DMA_HandleTypeDef* )hdma )->Parent;

  // Update state machine on conversion status if not in error state
  if ( HAL_IS_BIT_CLR( hadc->State, HAL_ADC_STATE_ERROR_INTERNAL | HAL_ADC_STATE_ERROR_DMA ) ) {
    // Update ADC state machine
    SET_BIT( hadc->State, HAL_ADC_STATE_REG_EOC );

    // Determine whether any further conversion upcoming on group regular
    // by external trigger, continuous mode or scan sequence on going.
    // Note: On STM32F4, there is no independent flag of end of sequence.
    //       The test of scan sequence on going is done either with scan
    //       sequence disabled or with end of conversion flag set to
    //       of end of sequence.
    if( ADC_IS_SOFTWARE_START_REGULAR( hadc )                   &&
       ( hadc->Init.ContinuousConvMode == DISABLE )            &&
       ( HAL_IS_BIT_CLR( hadc->Instance->SQR1, ADC_SQR1_L ) ||
        HAL_IS_BIT_CLR( hadc->Instance->CR2, ADC_CR2_EOCS )  )   )
    {
      // Disable ADC end of single conversion interrupt on group regular
      // Note: Overrun interrupt was enabled with EOC interrupt in
      // HAL_ADC_Start_IT( ), but is not disabled here because can be used
      // by overrun IRQ process below.
      __HAL_ADC_DISABLE_IT( hadc, ADC_IT_EOC );

      // Set ADC state
      CLEAR_BIT( hadc->State, HAL_ADC_STATE_REG_BUSY );

      if ( HAL_IS_BIT_CLR( hadc->State, HAL_ADC_STATE_INJ_BUSY ) ) {
        SET_BIT( hadc->State, HAL_ADC_STATE_READY );
      }
    }

    HAL_ADC_ConvCpltCallback( hadc );
  } else {
    hadc->DMA_Handle->XferErrorCallback( hdma );
  }
}

/**
  * @brief  DMA half transfer complete callback.
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
static void ADC_DMAHalfConvCplt_n( DMA_HandleTypeDef *hdma )
{
  log_add( "ADC2" NL );
  ADC_HandleTypeDef* hadc = ( ADC_HandleTypeDef* )( ( DMA_HandleTypeDef* )hdma )->Parent;
  // Conversion complete callback
  HAL_ADC_ConvHalfCpltCallback( hadc );
}

static void ADC_DMAError_n( DMA_HandleTypeDef *hdma )
{
  log_add( "ADCE" NL );
  ADC_HandleTypeDef* hadc = ( ADC_HandleTypeDef* )( (DMA_HandleTypeDef* )hdma )->Parent;
  hadc->State= HAL_ADC_STATE_ERROR_DMA;
  // Set ADC error code to DMA error
  hadc->ErrorCode |= HAL_ADC_ERROR_DMA;
  HAL_ADC_ErrorCallback( hadc );
}

