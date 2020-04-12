#include <oxc_adc.h>
// #include <oxc_auto.h> // tmp: debug, remove with std_out
// #include <oxc_debug1.h> // tmp: log_add
/**
  ******************************************************************************
  * @file    oxc_arch_adcdma_n.cpp, based on  stm32f7xx_hal_adc.c
  *          by  MCD Application Team
   ------------------------------------------------------------------*/


static void ADC_DMAConvCplt_c_n( DMA_HandleTypeDef *hdma ); // common part
static void ADC_DMAConvCplt_n( DMA_HandleTypeDef *hdma );
static void ADC_DMAM1Cplt_n( DMA_HandleTypeDef *hdma );

static void ADC_DMAHalfConvCplt_c_n( DMA_HandleTypeDef *hdma ); // common part
static void ADC_DMAHalfConvCplt_n( DMA_HandleTypeDef *hdma );
static void ADC_DMAM1HalfConvCplt_n( DMA_HandleTypeDef *hdma );

static void ADC_DMAError_n( DMA_HandleTypeDef *hdma );


HAL_StatusTypeDef ADC_Start_DMA_n( ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length, uint32_t chunkLength )
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  __IO uint32_t counter = 0;

  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(hadc->Init.ContinuousConvMode));
  assert_param(IS_ADC_EXT_TRIG_EDGE(hadc->Init.ExternalTrigConvEdge));

  /* Process locked */
  __HAL_LOCK(hadc);

  /* Enable the ADC peripheral */
  /* Check if ADC peripheral is disabled in order to enable it and wait during
     Tstab time the ADC's stabilization */
  if((hadc->Instance->CR2 & ADC_CR2_ADON) != ADC_CR2_ADON)
  {
    /* Enable the Peripheral */
    __HAL_ADC_ENABLE(hadc);

    /* Delay for ADC stabilization time */
    /* Compute number of CPU cycles to wait for */
    counter = (ADC_STAB_DELAY_US * (SystemCoreClock / 1000000));
    while(counter != 0)
    {
      counter--;
    }
  }

  /* Start conversion if ADC is effectively enabled */
  if(HAL_IS_BIT_SET(hadc->Instance->CR2, ADC_CR2_ADON))
  {
    /* Set ADC state                                                          */
    /* - Clear state bitfield related to regular group conversion results     */
    /* - Set state bitfield related to regular group operation                */
    ADC_STATE_CLR_SET(hadc->State,
                      HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC | HAL_ADC_STATE_REG_OVR,
                      HAL_ADC_STATE_REG_BUSY);

    /* If conversions on group regular are also triggering group injected,    */
    /* update ADC state.                                                      */
    if (READ_BIT(hadc->Instance->CR1, ADC_CR1_JAUTO) != RESET)
    {
      ADC_STATE_CLR_SET(hadc->State, HAL_ADC_STATE_INJ_EOC, HAL_ADC_STATE_INJ_BUSY);
    }

    /* State machine update: Check if an injected conversion is ongoing */
    if (HAL_IS_BIT_SET(hadc->State, HAL_ADC_STATE_INJ_BUSY))
    {
      /* Reset ADC error code fields related to conversions on group regular */
      CLEAR_BIT(hadc->ErrorCode, (HAL_ADC_ERROR_OVR | HAL_ADC_ERROR_DMA));
    }
    else
    {
      /* Reset ADC all error code fields */
      ADC_CLEAR_ERRORCODE(hadc);
    }

    /* Process unlocked */
    /* Unlock before starting ADC conversions: in case of potential           */
    /* interruption, to let the process to ADC IRQ Handler.                   */
    __HAL_UNLOCK(hadc);

    // atu: my callbacks here
    /* Set the DMA transfer complete callback */
    hadc->DMA_Handle->XferCpltCallback = ADC_DMAConvCplt_n;

    /* Set the DMA half transfer complete callback */
    hadc->DMA_Handle->XferHalfCpltCallback = ADC_DMAHalfConvCplt_n;

    /* Set the DMA error callback */
    hadc->DMA_Handle->XferErrorCallback = ADC_DMAError_n;
    // atu:
    hadc->DMA_Handle->XferM1CpltCallback = ADC_DMAM1Cplt_n;
    hadc->DMA_Handle->XferM1HalfCpltCallback = ADC_DMAM1HalfConvCplt_n;


    /* Manage ADC and DMA start: ADC overrun interruption, DMA start, ADC     */
    /* start (in case of SW start):                                           */

    /* Clear regular group conversion flag and overrun flag */
    /* (To ensure of no unknown state from potential previous ADC operations) */
    __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_EOC | ADC_FLAG_OVR);

    /* Enable ADC overrun interrupt */
    __HAL_ADC_ENABLE_IT(hadc, ADC_IT_OVR);

    /* Enable ADC DMA mode */
    hadc->Instance->CR2 |= ADC_CR2_DMA;


    // atu:
    uint32_t dat1 = (uint32_t)(pData);
    uint32_t dat2 = dat1 + chunkLength; //  + 8; // 8 is for debug:  one line of 4 samples
    adcdma_n_status.base = dat1;
    adcdma_n_status.next  = chunkLength;
    adcdma_n_status.step = chunkLength;
    uint32_t len_bytes = 2 * Length;
    adcdma_n_status.total_sz = len_bytes; // incoming length is in samples?
    uint32_t l_lim = ( len_bytes > chunkLength ) ? chunkLength : len_bytes;
    l_lim /= 2; // in elements


    // std_out << "# debug init:  "
    //   << " base= "     << HexInt(adcdma_n_status.base)
    //   << " next= "     << adcdma_n_status.next
    //   << " step= "     << adcdma_n_status.step
    //   << " total_sz= " << adcdma_n_status.total_sz
    //   << " l_lim= "    << l_lim << NL;

        ((DMA_Stream_TypeDef *)(hadc->DMA_Handle->Instance))->CR &= ~DMA_SxCR_CT; // atu: not auto cleard by init??
    /* Start the DMA channel atu: 2 buffer */
    tmp_hal_status = HAL_DMAEx_MultiBufferStart_IT( hadc->DMA_Handle, (uint32_t)&hadc->Instance->DR, dat1, dat2, l_lim );
    if( tmp_hal_status != HAL_OK ) {
      adcdma_n_status.reset();
      // UVAR('q') = tmp_hal_status;
      // error?
    }

    /* Check if Multimode enabled */
    if(HAL_IS_BIT_CLR(ADC->CCR, ADC_CCR_MULTI))
    {
      /* if no external trigger present enable software conversion of regular channels */
      if((hadc->Instance->CR2 & ADC_CR2_EXTEN) == RESET)
      {
        /* Enable the selected ADC software conversion for regular group */
        hadc->Instance->CR2 |= (uint32_t)ADC_CR2_SWSTART;
      }
    }
    else
    {
      /* if instance of handle correspond to ADC1 and  no external trigger present enable software conversion of regular channels */
      if((hadc->Instance == ADC1) && ((hadc->Instance->CR2 & ADC_CR2_EXTEN) == RESET))
      {
        /* Enable the selected ADC software conversion for regular group */
          hadc->Instance->CR2 |= (uint32_t)ADC_CR2_SWSTART;
      }
      /* if dual mode is selected, ADC3 works independently. */
      /* check if the mode selected is not triple */
      if( HAL_IS_BIT_CLR(ADC->CCR, ADC_CCR_MULTI_4) )
      {
        /* if instance of handle correspond to ADC3 and  no external trigger present enable software conversion of regular channels */
        if((hadc->Instance == ADC3) && ((hadc->Instance->CR2 & ADC_CR2_EXTEN) == RESET))
        {
          /* Enable the selected ADC software conversion for regular group */
          hadc->Instance->CR2 |= (uint32_t)ADC_CR2_SWSTART;
        }
      }
    }
  }

  /* Return function status */
  return tmp_hal_status;
}



/**
  * @brief  DMA transfer complete callback.
  * @param hdma pointer to DMA handle.
  * @retval None
  */
void ADC_DMAConvCplt_c_n( DMA_HandleTypeDef *hdma )
{
  /* Retrieve ADC handle corresponding to current DMA handle */
  ADC_HandleTypeDef* hadc = ( ADC_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;

  // atu:
  // log_add( "c.ccx " );
  bool is_final = adcdma_n_status.next > adcdma_n_status.total_sz;
  if( is_final ) {
    // log_add( "fin "  );
    HAL_ADC_Stop_DMA( hadc );
    adcdma_n_status.reset(); // and signal to stop
  }

  /* Update state machine on conversion status if not in error state */
  if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL | HAL_ADC_STATE_ERROR_DMA))
  {
    /* Update ADC state machine */
    SET_BIT(hadc->State, HAL_ADC_STATE_REG_EOC);

    /* Determine whether any further conversion upcoming on group regular   */
    /* by external trigger, continuous mode or scan sequence on going.      */
    /* Note: On STM32F7, there is no independent flag of end of sequence.   */
    /*       The test of scan sequence on going is done either with scan    */
    /*       sequence disabled or with end of conversion flag set to        */
    /*       of end of sequence.                                            */
    if(ADC_IS_SOFTWARE_START_REGULAR(hadc)                   &&
       (hadc->Init.ContinuousConvMode == DISABLE)            &&
       (HAL_IS_BIT_CLR(hadc->Instance->SQR1, ADC_SQR1_L) ||
        HAL_IS_BIT_CLR(hadc->Instance->CR2, ADC_CR2_EOCS)  )   )
    {
      /* Disable ADC end of single conversion interrupt on group regular */
      /* Note: Overrun interrupt was enabled with EOC interrupt in          */
      /* HAL_ADC_Start_IT(), but is not disabled here because can be used   */
      /* by overrun IRQ process below.                                      */
      __HAL_ADC_DISABLE_IT(hadc, ADC_IT_EOC);

      /* Set ADC state */
      CLEAR_BIT(hadc->State, HAL_ADC_STATE_REG_BUSY);

      if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_INJ_BUSY))
      {
        SET_BIT(hadc->State, HAL_ADC_STATE_READY);
      }
    }

    /* Conversion complete callback */
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
    hadc->ConvCpltCallback(hadc);
#else
    HAL_ADC_ConvCpltCallback(hadc);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
  }
  else /* DMA and-or internal error occurred */
  {
    if ((hadc->State & HAL_ADC_STATE_ERROR_INTERNAL) != 0UL)
    {
      /* Call HAL ADC Error Callback function */
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
      hadc->ErrorCallback(hadc);
#else
      HAL_ADC_ErrorCallback(hadc);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
    }
    else
    {
      /* Call DMA error callback */
      hadc->DMA_Handle->XferErrorCallback(hdma);
    }
  }
}

void ADC_DMAConvCplt_n( DMA_HandleTypeDef *hdma )
{
  // log_add( "c.cc1 " ); // really 1, logic in IRQ handler is inconsistent

  // bool is_mem1 = ((DMA_Stream_TypeDef *)(hdma->Instance))->CR & DMA_SxCR_CT;
  adcdma_n_status.makeStep();
  ((DMA_Stream_TypeDef *)(hdma->Instance))->M0AR = adcdma_n_status.base + adcdma_n_status.next;

  ADC_DMAConvCplt_c_n( hdma );
}

void ADC_DMAM1Cplt_n( DMA_HandleTypeDef *hdma )
{
  // log_add( "c.cc0 " ); // really 0
  adcdma_n_status.makeStep();
  ((DMA_Stream_TypeDef *)(hdma->Instance))->M1AR = adcdma_n_status.base + adcdma_n_status.next;
  ADC_DMAConvCplt_c_n( hdma );
}

/**
  * @brief  DMA half transfer complete callback.
  * @param hdma pointer to DMA handle.
  * @retval None
  */
void ADC_DMAHalfConvCplt_c_n( DMA_HandleTypeDef *hdma )
{
  /* Retrieve ADC handle corresponding to current DMA handle */
  ADC_HandleTypeDef *hadc = (ADC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  // log_add( "c.hcx " );

  /* Half conversion callback */
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
  hadc->ConvHalfCpltCallback(hadc);
#else
  HAL_ADC_ConvHalfCpltCallback(hadc);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
}

void ADC_DMAHalfConvCplt_n( DMA_HandleTypeDef *hdma )
{
  // log_add( "c.hc0 " );
  // ((DMA_Stream_TypeDef *)(hdma->Instance))->M1AR = adcdma_n_status.base + adcdma_n_status.next; // keep
  ADC_DMAHalfConvCplt_c_n( hdma );
}

void ADC_DMAM1HalfConvCplt_n( DMA_HandleTypeDef *hdma )
{
  // log_add( "c.hc1 " );
  // ((DMA_Stream_TypeDef *)(hdma->Instance))->M0AR = adcdma_n_status.base + adcdma_n_status.next;
  ADC_DMAHalfConvCplt_c_n( hdma );
}


/**
  * @brief  DMA error callback.
  * @param hdma pointer to DMA handle.
  * @retval None
  */
void ADC_DMAError_n( DMA_HandleTypeDef *hdma )
{
  /* Retrieve ADC handle corresponding to current DMA handle */
  ADC_HandleTypeDef *hadc = (ADC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  // log_add( "c.er " );

  /* Set ADC state */
  SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_DMA);

  /* Set ADC error code to DMA error */
  SET_BIT(hadc->ErrorCode, HAL_ADC_ERROR_DMA);

  /* Error callback */
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
  hadc->ErrorCallback(hadc);
#else
  HAL_ADC_ErrorCallback(hadc);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
}


