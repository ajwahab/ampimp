
#include "main.h"
#include "Impedance.h"
#include "Amperometric.h"

enum app_id {APP_ID_AMP, APP_ID_IMP, APP_NUM};

#define APP_AMP_SEQ_ADDR    (0)
#define APP_IMP_SEQ_ADDR    (256)

#define APP_AMP_MAX_SEQLEN  (256)
#define APP_IMP_MAX_SEQLEN  (128)

typedef struct
{
  AD5940Err (*pAppGetCfg) (void *pCfg);
  AD5940Err (*pAppInit)   (uint32_t *pBuffer, uint32_t BufferSize);
  AD5940Err (*pAppISR)    (void *pBuff, uint32_t *pCount);
  AD5940Err (*pAppCtrl)   (int32_t BcmCtrl, void *pPara);
  AD5940Err (*pAppUserDataProc)    (void *pBuff, uint32_t pCount);
} app_t;

AD5940Err AmpShowResult(float *pData, uint32_t DataCount);
AD5940Err ImpShowResult(uint32_t *pData, uint32_t DataCount);

app_t app_list[APP_NUM] =
{
  /* Amperometric App */
  {
    .pAppGetCfg = AppAMPGetCfg,
    .pAppInit = AppAMPInit,
    .pAppISR = AppAMPISR,
    .pAppCtrl = AppAMPCtrl,
    .pAppUserDataProc = AmpShowResult,
  },
  /* Impedance App */
  {
    .pAppGetCfg = AppIMPGetCfg,
    .pAppInit = AppIMPInit,
    .pAppISR = AppIMPISR,
    .pAppCtrl = AppIMPCtrl,
    .pAppUserDataProc = ImpShowResult,
  }
};

AD5940Err AmpShowResult(float *pData, uint32_t DataCount)
{
  /* Print data*/
  for(int i=0;i<DataCount;i++)
  {
    printf("index:%d, Current:%fuA\r\n", i, pData[i]);
  }
  return 0;
}

AD5940Err ImpShowResult(uint32_t *pData, uint32_t DataCount)
{
  float freq;

  fImpPol_Type *pImp = (fImpPol_Type*)pData;
  AppIMPCtrl(IMPCTRL_GETFREQ, &freq);

  printf("Freq:%.2f \r\n", freq);
  /*Process data*/
  for(int i=0;i<DataCount;i++)
  {
    printf("RzMag: %f Ohm, RzPhase: %f \r\n", pImp[i].Magnitude, pImp[i].Phase*180/MATH_PI);
  }
  return 0;
}

#define APP_BUFF_SIZE   (512)
uint32_t AppBuff[APP_BUFF_SIZE];
float LFOSCFreq;    /* Measured LFOSC frequency */

/* Initialize AD5940 basic blocks like clock */
static int32_t AD5940PlatformCfg(void)
{
  CLKCfg_Type clk_cfg;
  FIFOCfg_Type fifo_cfg;
  SEQCfg_Type seq_cfg;
  AGPIOCfg_Type gpio_cfg;
  LFOSCMeasure_Type LfoscMeasure;

  /* Use hardware reset */
  AD5940_HWReset();
  /* Platform configuration */
  AD5940_Initialize();
  /* Step1. Configure clock */
  clk_cfg.ADCClkDiv = ADCCLKDIV_1;
  clk_cfg.ADCCLkSrc = ADCCLKSRC_HFOSC;
  clk_cfg.SysClkDiv = SYSCLKDIV_1;
  clk_cfg.SysClkSrc = SYSCLKSRC_HFOSC;
  clk_cfg.HfOSC32MHzMode = bFALSE;
  clk_cfg.HFOSCEn = bTRUE;
  clk_cfg.HFXTALEn = bFALSE;
  clk_cfg.LFOSCEn = bTRUE;
  AD5940_CLKCfg(&clk_cfg);
  /* Step2. Configure FIFO and Sequencer*/
  fifo_cfg.FIFOEn = bFALSE;
  fifo_cfg.FIFOMode = FIFOMODE_FIFO;
  fifo_cfg.FIFOSize = FIFOSIZE_4KB;                       /* 4kB for FIFO, The reset 2kB for sequencer */
  fifo_cfg.FIFOSrc = FIFOSRC_DFT;
  fifo_cfg.FIFOThresh = 4;//AppBIACfg.FifoThresh;        /* DFT result. One pair for RCAL, another for Rz. One DFT result have real part and imaginary part */
  AD5940_FIFOCfg(&fifo_cfg);                             /* Disable to reset FIFO. */
  fifo_cfg.FIFOEn = bTRUE;
  AD5940_FIFOCfg(&fifo_cfg);                             /* Enable FIFO here */
  /* Configure sequencer and stop it */
  seq_cfg.SeqMemSize = SEQMEMSIZE_2KB;
  seq_cfg.SeqBreakEn = bFALSE;
  seq_cfg.SeqIgnoreEn = bFALSE;
  seq_cfg.SeqCntCRCClr = bTRUE;
  seq_cfg.SeqEnable = bFALSE;
  seq_cfg.SeqWrTimer = 0;
  AD5940_SEQCfg(&seq_cfg);

  /* Step3. Interrupt controller */
  AD5940_INTCCfg(AFEINTC_1, AFEINTSRC_ALLINT, bTRUE);           /* Enable all interrupt in Interrupt Controller 1, so we can check INTC flags */
  AD5940_INTCCfg(AFEINTC_0, AFEINTSRC_DATAFIFOTHRESH, bTRUE);   /* Interrupt Controller 0 will control GP0 to generate interrupt to MCU */
  AD5940_INTCClrFlag(AFEINTSRC_ALLINT);
  /* Step4: Reconfigure GPIO */
  gpio_cfg.FuncSet = GP6_SYNC|GP5_SYNC|GP4_SYNC|GP2_EXTCLK|GP1_SYNC|GP0_INT;
  gpio_cfg.InputEnSet = AGPIO_Pin2;
  gpio_cfg.OutputEnSet = AGPIO_Pin0|AGPIO_Pin1|AGPIO_Pin4|AGPIO_Pin5|AGPIO_Pin6;
  gpio_cfg.OutVal = 0;
  gpio_cfg.PullEnSet = 0;
  AD5940_AGPIOCfg(&gpio_cfg);

  AD5940_SleepKeyCtrlS(SLPKEY_UNLOCK);  /* Enable AFE to enter sleep mode. */

  /* Measure LFOSC frequency */
  LfoscMeasure.CalDuration = 1000.0;  /* 1000ms used for calibration. */
  LfoscMeasure.CalSeqAddr = 0;
  LfoscMeasure.SystemClkFreq = 16000000.0f; /* 16MHz in this firmware. */
  AD5940_LFOSCMeasure(&LfoscMeasure, &LFOSCFreq);
  printf("LFOSCFreq: %f\r\n", LFOSCFreq);
  return 0;
}

void AD5940AMPStructInit(void)
{
  AppAMPCfg_Type *pCfg;

  AppAMPGetCfg(&pCfg);
  pCfg->WuptClkFreq = LFOSCFreq;
  /* Configure general parameters */
  pCfg->SeqStartAddr = 0;
  pCfg->MaxSeqLen = 512;     /* @todo add checker in function */
  pCfg->RcalVal = 10000.0;
  pCfg->NumOfData = -1;      /* Never stop until you stop it manually by AppAMPCtrl() function */

  /* Configure measurement parameters */
  pCfg->AmpODR = 1;            /* Time between samples in seconds */
  pCfg->FifoThresh = 4;          /* Number of measurements before alerting host microcontroller */

  pCfg->SensorBias = 0;        /* Sensor bias voltage between reference and sense electrodes*/
  pCfg->LptiaRtiaSel = LPTIARTIA_1K;
  pCfg->LpTiaRl = LPTIARLOAD_10R;
  pCfg->Vzero = 1100;            /* Vzero voltage. Voltage on Sense electrode. Unit is mV*/

  pCfg->ADCRefVolt = 1.82;   /* Measure voltage on Vref_1V8 pin */
}

void AD5940IMPStructInit(void)
{
  AppIMPCfg_Type *pCfg;

  AppIMPGetCfg(&pCfg);
  /* Step1: configure initialization sequence Info */
  pCfg->SeqStartAddr = 0;
  pCfg->MaxSeqLen = 512; /* @todo add checker in function */

  pCfg->RcalVal = 10000.0;
  pCfg->SinFreq = 60000.0;
  pCfg->FifoThresh = 4;

  /* Set switch matrix to onboard(EVAL-AD5940ELECZ) dummy sensor. */
  /* Note the RCAL0 resistor is 10kOhm. */
  pCfg->DswitchSel = SWD_CE0;
  pCfg->PswitchSel = SWP_RE0;
  pCfg->NswitchSel = SWN_SE0;
  pCfg->TswitchSel = SWT_SE0LOAD;
  /* The dummy sensor is as low as 5kOhm. We need to make sure RTIA is small enough that HSTIA won't be saturated. */
  pCfg->HstiaRtiaSel = HSTIARTIA_5K;

  /* Configure the sweep function. */
  pCfg->SweepCfg.SweepEn = bTRUE;
  pCfg->SweepCfg.SweepStart = 100.0f;  /* Start from 1kHz */
  pCfg->SweepCfg.SweepStop = 100e3f;   /* Stop at 100kHz */
  pCfg->SweepCfg.SweepPoints = 101;    /* Points is 101 */
  pCfg->SweepCfg.SweepLog = bTRUE;
  /* Configure Power Mode. Use HP mode if frequency is higher than 80kHz. */
  pCfg->PwrMod = AFEPWR_HP;
  /* Configure filters if necessary */
  pCfg->ADCSinc3Osr = ADCSINC3OSR_2;   /* Sample rate is 800kSPS/2 = 400kSPS */
  pCfg->DftNum = DFTNUM_16384;
  pCfg->DftSrc = DFTSRC_SINC3;
}

app_t *pCurrApp;
uint8_t bSwitchApp = 1;
uint8_t toApp = APP_ID_AMP;

void AD5940_Main(void)
{
  static uint32_t IntCount;
  uint32_t temp;

  AD5940PlatformCfg();

  AD5940AMPStructInit();
  AD5940IMPStructInit();

  while(1)
  {
    if(bSwitchApp)
    {
      bSwitchApp = 0;
      pCurrApp = &app_list[toApp];
      /* Initialize registers that fit to all measurements */
      AD5940PlatformCfg();
      pCurrApp->pAppInit(AppBuff, APP_BUFF_SIZE);
      AD5940_ClrMCUIntFlag(); /* Clear the interrupts happened during initialization */
      pCurrApp->pAppCtrl(APPCTRL_START, 0);         /* Control BIA measurment to start. Second parameter has no meaning with this command. */
    }
    /* Check if interrupt flag which will be set when interrupt occured. */
    if(AD5940_GetMCUIntFlag())
    {
      AD5940_ClrMCUIntFlag(); /* Clear this flag */
      temp = APP_BUFF_SIZE;
      pCurrApp->pAppISR(AppBuff, &temp); /* Deal with it and provide a buffer to store data we got */
      if(pCurrApp->pAppUserDataProc)
        pCurrApp->pAppUserDataProc(AppBuff, temp); /* Show the results to UART */

      if(IntCount++ == 10)
      {
        IntCount = 0;
        /* Control the application at any time */
        /* For example, I want to measure EDA excitation voltage periodically */
        //if(toApp == APP_ID_EDA)
        //  pCurrApp->pAppCtrl(EDACTRL_MEASVOLT, 0);
      }
    }
  }
}

uint32_t command_start_measurment(uint32_t para1, uint32_t para2)
{
  pCurrApp->pAppCtrl(APPCTRL_START, 0);
  return 0;
}

uint32_t command_stop_measurment(uint32_t para1, uint32_t para2)
{
  pCurrApp->pAppCtrl(APPCTRL_STOPNOW, 0);
  return 0;
}

uint32_t command_switch_app(uint32_t AppID, uint32_t para2)
{
  if(AppID == APP_ID_AMP)
  {
    AD5940AMPStructInit();
    printf("Switch to Amperometric application\r\n");
  }
  else if(AppID == APP_ID_IMP)
  {
    AD5940IMPStructInit();
    printf("Switch to Impedance application\r\n");
  }
  else {
    printf("Invalid application ID.\r\n");
    return (uint32_t)-1;
  }

  if(pCurrApp)
    pCurrApp->pAppCtrl(APPCTRL_STOPNOW, 0);
  bSwitchApp = 1;
  toApp = AppID;
  return 0;
}
