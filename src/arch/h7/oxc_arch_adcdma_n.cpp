#include <oxc_adc.h>
#include <oxc_auto.h> // tmp: debug, remove with std_out
#include <oxc_debug1.h> // tmp: log_add
/**
  ******************************************************************************
  * @file    oxc_arch_adcdma_n.cpp, based on  stm32h7xx_hal_adc.c
  *          by  MCD Application Team
   ------------------------------------------------------------------*/

AdcDma_n_status adcdma_n_status;

static void ADC_DMAConvCplt_c_n( DMA_HandleTypeDef *hdma ); // common part
static void ADC_DMAConvCplt_n( DMA_HandleTypeDef *hdma );
static void ADC_DMAM1Cplt_n( DMA_HandleTypeDef *hdma );

static void ADC_DMAHalfConvCplt_c_n( DMA_HandleTypeDef *hdma ); // common part
static void ADC_DMAHalfConvCplt_n( DMA_HandleTypeDef *hdma );
static void ADC_DMAM1HalfConvCplt_n( DMA_HandleTypeDef *hdma );

static void ADC_DMAError_n( DMA_HandleTypeDef *hdma );


HAL_StatusTypeDef ADC_Start_DMA_n( ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length, uint32_t chunkLength )
{
  HAL_StatusTypeDef tmp_hal_status;
  uint32_t tmp_multimode_config = LL_ADC_GetMultimode(__LL_ADC_COMMON_INSTANCE(hadc->Instance));

  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));

  /* Perform ADC enable and conversion start if no conversion is on going */
  if (LL_ADC_REG_IsConversionOngoing(hadc->Instance) == 0UL)
  {
    /* Process locked */
    __HAL_LOCK(hadc);

    /* Ensure that multimode regular conversions are not enabled.   */
    /* Otherwise, dedicated API HAL_ADCEx_MultiModeStart_DMA() must be used.  */
    if ((tmp_multimode_config == LL_ADC_MULTI_INDEPENDENT)
        || (tmp_multimode_config == LL_ADC_MULTI_DUAL_INJ_SIMULT)
        || (tmp_multimode_config == LL_ADC_MULTI_DUAL_INJ_ALTERN)
       )
    {
      /* Enable the ADC peripheral */
      tmp_hal_status = ADC_Enable(hadc);

      /* Start conversion if ADC is effectively enabled */
      if (tmp_hal_status == HAL_OK)
      {
        /* Set ADC state                                                        */
        /* - Clear state bitfield related to regular group conversion results   */
        /* - Set state bitfield related to regular operation                    */
        ADC_STATE_CLR_SET(hadc->State,
                          HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC | HAL_ADC_STATE_REG_OVR | HAL_ADC_STATE_REG_EOSMP,
                          HAL_ADC_STATE_REG_BUSY);

        /* Reset HAL_ADC_STATE_MULTIMODE_SLAVE bit
          - if ADC instance is master or if multimode feature is not available
          - if multimode setting is disabled (ADC instance slave in independent mode) */
        if ((__LL_ADC_MULTI_INSTANCE_MASTER(hadc->Instance) == hadc->Instance)
            || (tmp_multimode_config == LL_ADC_MULTI_INDEPENDENT)
           )
        {
          CLEAR_BIT(hadc->State, HAL_ADC_STATE_MULTIMODE_SLAVE);
        }

        /* Check if a conversion is on going on ADC group injected */
        if ((hadc->State & HAL_ADC_STATE_INJ_BUSY) != 0UL)
        {
          /* Reset ADC error code fields related to regular conversions only */
          CLEAR_BIT(hadc->ErrorCode, (HAL_ADC_ERROR_OVR | HAL_ADC_ERROR_DMA));
        }
        else
        {
          /* Reset all ADC error code fields */
          ADC_CLEAR_ERRORCODE(hadc);
        }

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


        /* Manage ADC and DMA start: ADC overrun interruption, DMA start,     */
        /* ADC start (in case of SW start):                                   */

        /* Clear regular group conversion flag and overrun flag               */
        /* (To ensure of no unknown state from potential previous ADC         */
        /* operations)                                                        */
        __HAL_ADC_CLEAR_FLAG(hadc, (ADC_FLAG_EOC | ADC_FLAG_EOS | ADC_FLAG_OVR));

        /* Process unlocked */
        /* Unlock before starting ADC conversions: in case of potential         */
        /* interruption, to let the process to ADC IRQ Handler.                 */
        __HAL_UNLOCK(hadc);

        /* With DMA, overrun event is always considered as an error even if
           hadc->Init.Overrun is set to ADC_OVR_DATA_OVERWRITTEN. Therefore,
           ADC_IT_OVR is enabled. */
        __HAL_ADC_ENABLE_IT(hadc, ADC_IT_OVR);

        /* Enable ADC DMA  mode*/
        LL_ADC_REG_SetDataTransferMode(hadc->Instance, (uint32_t)hadc->Init.ConversionDataManagement);

        // atu:
        uint32_t dat1 = (uint32_t)(pData);
        uint32_t dat2 = dat1 + chunkLength; //  + 8; // 8 is for debug:  one line of 4 samples
        adcdma_n_status.base = dat1;
        adcdma_n_status.next  = chunkLength;
        adcdma_n_status.step = chunkLength;
        adcdma_n_status.total_sz = Length;
        uint32_t l_lim = ( Length > chunkLength ) ? chunkLength : Length;
        l_lim /= 2; // in elements


        std_out << "# debug init:  "
          << " base= "     << HexInt(adcdma_n_status.base)
          << " next= "     << adcdma_n_status.next
          << " step= "     << adcdma_n_status.step
          << " total_sz= " << adcdma_n_status.total_sz
          << " l_lim= "    << l_lim << NL;

        // ((DMA_Stream_TypeDef *)(hadc->DMA_Handle->Instance))->CR &= ~DMA_SxCR_CT; // atu: ??????????????????

        /* Start the DMA channel atu: 2 buffer */
        tmp_hal_status = HAL_DMAEx_MultiBufferStart_IT( hadc->DMA_Handle, (uint32_t)&hadc->Instance->DR, dat1, dat2, l_lim );
        if( tmp_hal_status != HAL_OK ) {
          adcdma_n_status.reset();
          UVAR('q') = tmp_hal_status;
          // error?
        }

        /* Enable conversion of regular group.                                  */
        /* If software start has been selected, conversion starts immediately.  */
        /* If external trigger has been selected, conversion will start at next */
        /* trigger event.                                                       */
        /* Start ADC group regular conversion */
        LL_ADC_REG_StartConversion(hadc->Instance);
      }
      else
      {
        /* Process unlocked */
        __HAL_UNLOCK(hadc);
      }

    }
    else
    {
      tmp_hal_status = HAL_ERROR;
      /* Process unlocked */
      __HAL_UNLOCK(hadc);
    }
  }
  else
  {
    tmp_hal_status = HAL_BUSY;
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
  ADC_HandleTypeDef *hadc = (ADC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  // atu:
  log_add( "c.ccx " );
  bool is_final = adcdma_n_status.next > adcdma_n_status.total_sz;
  if( is_final ) {
    log_add( "fin "  );
    HAL_ADC_Stop_DMA( hadc );
    adcdma_n_status.reset();
  }

  /* Update state machine on conversion status if not in error state */
  if ((hadc->State & (HAL_ADC_STATE_ERROR_INTERNAL | HAL_ADC_STATE_ERROR_DMA)) == 0UL)
  {
    /* Set ADC state */
    SET_BIT(hadc->State, HAL_ADC_STATE_REG_EOC);

    /* Determine whether any further conversion upcoming on group regular     */
    /* by external trigger, continuous mode or scan sequence on going         */
    /* to disable interruption.                                               */
    /* Is it the end of the regular sequence ? */
    if ((hadc->Instance->ISR & ADC_FLAG_EOS) != 0UL)
    {
      /* Are conversions software-triggered ? */
      if (LL_ADC_REG_IsTriggerSourceSWStart(hadc->Instance) != 0UL)
      {
        /* Is CONT bit set ? */
        if (READ_BIT(hadc->Instance->CFGR, ADC_CFGR_CONT) == 0UL)
        {
          /* CONT bit is not set, no more conversions expected */
          CLEAR_BIT(hadc->State, HAL_ADC_STATE_REG_BUSY);
          if ((hadc->State & HAL_ADC_STATE_INJ_BUSY) == 0UL)
          {
            SET_BIT(hadc->State, HAL_ADC_STATE_READY);
          }
        }
      }
    }
    else
    {
      /* DMA End of Transfer interrupt was triggered but conversions sequence
         is not over. If DMACFG is set to 0, conversions are stopped. */
      if (READ_BIT(hadc->Instance->CFGR, ADC_CFGR_DMNGT) == 0UL)
      {
        /* DMACFG bit is not set, conversions are stopped. */
        CLEAR_BIT(hadc->State, HAL_ADC_STATE_REG_BUSY);
        if ((hadc->State & HAL_ADC_STATE_INJ_BUSY) == 0UL)
        {
          SET_BIT(hadc->State, HAL_ADC_STATE_READY);
        }
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
      /* Call ADC DMA error callback */
      hadc->DMA_Handle->XferErrorCallback(hdma);
    }
  }
}

void ADC_DMAConvCplt_n( DMA_HandleTypeDef *hdma )
{
  log_add( "c.cc1 " ); // really 1, logic in IRQ handler is inconsistent

  // bool is_mem1 = ((DMA_Stream_TypeDef *)(hdma->Instance))->CR & DMA_SxCR_CT;
  adcdma_n_status.makeStep();
  ((DMA_Stream_TypeDef *)(hdma->Instance))->M0AR = adcdma_n_status.base + adcdma_n_status.next;

  ADC_DMAConvCplt_c_n( hdma );
}

void ADC_DMAM1Cplt_n( DMA_HandleTypeDef *hdma )
{
  log_add( "c.cc0 " ); // really 0
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

  log_add( "c.hcx " );

  /* Half conversion callback */
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
  hadc->ConvHalfCpltCallback(hadc);
#else
  HAL_ADC_ConvHalfCpltCallback(hadc);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
}

void ADC_DMAHalfConvCplt_n( DMA_HandleTypeDef *hdma )
{
  log_add( "c.hc0 " );
  adcdma_n_status.makeStep();
  ((DMA_Stream_TypeDef *)(hdma->Instance))->M1AR = adcdma_n_status.base + adcdma_n_status.next;
  ADC_DMAHalfConvCplt_c_n( hdma );
}

void ADC_DMAM1HalfConvCplt_n( DMA_HandleTypeDef *hdma )
{
  log_add( "c.hc1 " );
  adcdma_n_status.makeStep();
  uint32_t addr = adcdma_n_status.base + adcdma_n_status.next;
  UVAR('z') = 0;
  ((DMA_Stream_TypeDef *)(hdma->Instance))->M0AR = addr;
  // ((DMA_Stream_TypeDef *)(hdma->Instance))->M0AR = adcdma_n_status.base + adcdma_n_status.next;
  // ((DMA_Stream_TypeDef *)(hdma->Instance))->M0AR = 0x24004444;
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

  log_add( "c.er " );

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


