/**
 * @defgroup   AD5940MAIN
 *
 * @brief      Based on AD5940_Amperometric/AD5940Main.c, AD5940_Impedance/AD5940Main.c, and AD5940_BioElec/AD5940Main.c examples from Analog Devices, Inc.
 *
 * @author     Adam Wahab
 * @date       2020
 */

#include "main.h"
#include "Impedance.h" //in AD5940_Impedance
#include "Amperometric.h" //in AD5940_Amperometric

enum {
  APP_ID_AMP,
  APP_ID_IMP,
  APP_NUM
};

#define APP_AMP_SEQ_ADDR    (0)
#define APP_IMP_SEQ_ADDR    (256)

#define APP_AMP_MAX_SEQLEN  (256)
#define APP_IMP_MAX_SEQLEN  (128)

#define APP_BUF_SIZE        (512)

#define APP_READY_STR "\r\n> "
#define APP_LEAD_CHAR "$"
#define APP_TRAIL_CHAR "\r"

typedef struct
{
  uint32_t isr_count;
  uint32_t cycle_limit; //number of cycles to perform
  uint8_t app_id_next; //id of app to following current app
  AD5940Err (*pAppGetCfg) (void *p_cfg);
  AD5940Err (*pAppInit)   (uint32_t *p_buf, uint32_t buf_size);
  AD5940Err (*pAppISR)    (void *p_buf, uint32_t *p_count);
  AD5940Err (*pAppCtrl)   (int32_t bcm_ctrl, void *p_para);
  AD5940Err (*pAppUserDataProc)    (void *p_buf, uint32_t p_count);
} app_t;

AD5940Err amp_print_result(void *p_data, uint32_t data_count);
AD5940Err imp_print_result(void *p_data, uint32_t data_count);

app_t *g_p_app_curr; //pointer to object for currently running app
uint8_t g_switch_app = 1; //flag to allow app switching
uint8_t g_app_id = APP_ID_AMP; //set initial application
uint32_t g_app_buf[APP_BUF_SIZE];
float g_lfo_sc_freq;    /* Measured LFOSC frequency */

app_t g_app_list[APP_NUM] =
{
  /* Amperometric App */
  {
    .isr_count = 0,
    .cycle_limit = 10,
    .app_id_next = APP_ID_IMP,
    .pAppGetCfg = AppAMPGetCfg,
    .pAppInit = AppAMPInit,
    .pAppISR = AppAMPISR,
    .pAppCtrl = AppAMPCtrl,
    .pAppUserDataProc = amp_print_result
  },
  /* Impedance App */
  {
    .isr_count = 0,
    .cycle_limit = 1,
    .app_id_next = APP_ID_AMP,
    .pAppGetCfg = AppIMPGetCfg,
    .pAppInit = AppIMPInit,
    .pAppISR = AppIMPISR,
    .pAppCtrl = AppIMPCtrl,
    .pAppUserDataProc = imp_print_result
  }
};

/**
 * @brief      Based on AmpShowResult in AD5940_Amperometric
 *
 * @param      p_data      The data
 * @param[in]  data_count  The data count
 *
 * @return     Error flag
 */
AD5940Err amp_print_result(void *p_data, uint32_t data_count)
{
  float *pAmp = (float *)p_data;
  /* Print data*/
  for(int i=0; i<data_count; i++)
  {
    printf("%i,%i,%f\r\n", g_app_id, i, pAmp[i]);
  }
  return 0;
}

/**
 * @brief      Based on ImpShowResult in AD5940_Impedance
 *
 * @param      p_data      The data
 * @param[in]  data_count  The data count
 *
 * @return     Error flag
 */
AD5940Err imp_print_result(void *p_data, uint32_t data_count)
{
  float freq;
  fImpPol_Type *pImp = (fImpPol_Type*)p_data;
  AppIMPCtrl(IMPCTRL_GETFREQ, &freq);

  for(int i=0; i<data_count; i++)
  {
    // printf("RzMag: %f Ohm, RzPhase: %f\r\n", pImp[i].Magnitude, pImp[i].Phase*180/MATH_PI);
    printf("%i,%i,%f,%f,%f\r\n", g_app_id, i, freq, pImp[i].Magnitude, (pImp[i].Phase*180)/MATH_PI);
  }
  return 0;
}

/**
 * @brief      Initialize AD5940 basic blocks like clock.
 * Based on AD5940PlatformCfg in AD5940_BioElec
 *
 * @return     { description_of_the_return_value }
 */
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
  AD5940_LFOSCMeasure(&LfoscMeasure, &g_lfo_sc_freq);
  return 0;
}

void app_cfg_struct_init(uint8_t app_id)
{
  void *p_cfg = NULL;
  g_p_app_curr = &g_app_list[app_id];

  switch(app_id)
  {
    case APP_ID_AMP:
      g_p_app_curr->pAppGetCfg((AppAMPCfg_Type*)p_cfg);
      ((AppAMPCfg_Type*)p_cfg)->WuptClkFreq = g_lfo_sc_freq;
      /* Configure general parameters */
      ((AppAMPCfg_Type*)p_cfg)->SeqStartAddr = 0;
      ((AppAMPCfg_Type*)p_cfg)->MaxSeqLen = 512;     /* @todo add checker in function */
      ((AppAMPCfg_Type*)p_cfg)->RcalVal = 10000.0;
      ((AppAMPCfg_Type*)p_cfg)->NumOfData = -1;      /* Never stop until you stop it manually by AppAMPCtrl() function */
      /* Configure measurement parameters */
      ((AppAMPCfg_Type*)p_cfg)->AmpODR = 1;            /* Time between samples in seconds */
      ((AppAMPCfg_Type*)p_cfg)->FifoThresh = 4;          /* Number of measurements before alerting host microcontroller */
      ((AppAMPCfg_Type*)p_cfg)->SensorBias = 0;        /* Sensor bias voltage between reference and sense electrodes*/
      ((AppAMPCfg_Type*)p_cfg)->LptiaRtiaSel = LPTIARTIA_1K;
      ((AppAMPCfg_Type*)p_cfg)->LpTiaRl = LPTIARLOAD_10R;
      ((AppAMPCfg_Type*)p_cfg)->Vzero = 1100;            /* Vzero voltage. Voltage on Sense electrode. Unit is mV*/
      ((AppAMPCfg_Type*)p_cfg)->ADCRefVolt = 1.82;   /* Measure voltage on Vref_1V8 pin */
      break;
    case APP_ID_IMP:
      g_p_app_curr->pAppGetCfg((AppIMPCfg_Type*)p_cfg);
      /* Step1: configure initialization sequence Info */
      ((AppIMPCfg_Type*)p_cfg)->SeqStartAddr = 0;
      ((AppIMPCfg_Type*)p_cfg)->MaxSeqLen = 512; /* @todo add checker in function */
      ((AppIMPCfg_Type*)p_cfg)->RcalVal = 10000.0;
      ((AppIMPCfg_Type*)p_cfg)->SinFreq = 60000.0;
      ((AppIMPCfg_Type*)p_cfg)->FifoThresh = 4;
      /* Set switch matrix to onboard(EVAL-AD5940ELECZ) dummy sensor. */
      /* Note the RCAL0 resistor is 10kOhm. */
      ((AppIMPCfg_Type*)p_cfg)->DswitchSel = SWD_CE0;
      ((AppIMPCfg_Type*)p_cfg)->PswitchSel = SWP_RE0;
      ((AppIMPCfg_Type*)p_cfg)->NswitchSel = SWN_SE0;
      ((AppIMPCfg_Type*)p_cfg)->TswitchSel = SWT_SE0LOAD;
       /*The dummy sensor is as low as 5kOhm. We need to make sure RTIA is small enough that HSTIA won't be saturated. */
      ((AppIMPCfg_Type*)p_cfg)->HstiaRtiaSel = HSTIARTIA_5K;
      /* Configure the sweep function. */
      ((AppIMPCfg_Type*)p_cfg)->SweepCfg.SweepEn = bTRUE;
      ((AppIMPCfg_Type*)p_cfg)->SweepCfg.SweepStart = 100.0f;  /* Start from 1kHz */
      ((AppIMPCfg_Type*)p_cfg)->SweepCfg.SweepStop = 100e3f;   /* Stop at 100kHz */
      ((AppIMPCfg_Type*)p_cfg)->SweepCfg.SweepPoints = 101;    /* Points is 101 */
      ((AppIMPCfg_Type*)p_cfg)->SweepCfg.SweepLog = bTRUE;
      /* Configure Power Mode. Use HP mode if frequency is higher than 80kHz. */
      ((AppIMPCfg_Type*)p_cfg)->PwrMod = AFEPWR_HP;
      /* Configure filters if necessary */
      ((AppIMPCfg_Type*)p_cfg)->ADCSinc3Osr = ADCSINC3OSR_2;   /* Sample rate is 800kSPS/2 = 400kSPS */
      ((AppIMPCfg_Type*)p_cfg)->DftNum = DFTNUM_16384;
      ((AppIMPCfg_Type*)p_cfg)->DftSrc = DFTSRC_SINC3;
      break;
    default:
      break;
  }
}

void AD5940_Main(void)
{
  uint32_t temp;

  AD5940PlatformCfg();

  for(uint8_t i=0; i<APP_NUM; i++)
  {
    app_cfg_struct_init(i);
  }

  while(1)
  {
    if(g_switch_app)
    {
      g_switch_app = 0;
      g_p_app_curr = &g_app_list[g_app_id];
      AD5940PlatformCfg();
      g_p_app_curr->pAppInit(g_app_buf, APP_BUF_SIZE);
      AD5940_ClrMCUIntFlag();
      //cmd_start_measurment(); //start application
      printf(APP_READY_STR); //app ready
    }
    if(AD5940_GetMCUIntFlag())
    {
      AD5940_ClrMCUIntFlag();
      temp = APP_BUF_SIZE;
      g_p_app_curr->pAppISR(g_app_buf, &temp);
      if(g_p_app_curr->pAppUserDataProc)
        g_p_app_curr->pAppUserDataProc(g_app_buf, temp);

      /// Periodically switch applications based on count, e.g., measure imp. once after every 10th amp. measurement cycle.
      if((g_p_app_curr->isr_count++ >= g_p_app_curr->cycle_limit))
      {
        g_p_app_curr->isr_count = 0;
        cmd_switch_app((uint32_t)g_p_app_curr->app_id_next, 0);
      }
    }
  }
}

/**
 * @brief      Based on command_start_measurment from AD5940_BioElec
 *
 * @param[in]  para1  The para 1
 * @param[in]  para2  The para 2
 *
 * @return     0
 */
uint32_t cmd_start_measurment(uint32_t para1, uint32_t para2)
{
  g_p_app_curr->pAppCtrl(APP_CTRL_START, 0); //pointer to current app object
  return 0;
}

/**
 * @brief      Based on command_stop_measurment from AD5940_BioElec
 *
 * @param[in]  para1  The para 1
 * @param[in]  para2  The para 2
 *
 * @return     0
 */
uint32_t cmd_stop_measurment(uint32_t para1, uint32_t para2)
{
  g_p_app_curr->pAppCtrl(APP_CTRL_STOP_NOW, 0); //pointer to current app object
  return 0;
}

/**
 * @brief      Based on command_switch_app from AD5940_BioElec
 *
 * @param[in]  para1  The para 1
 * @param[in]  para2  The para 2
 *
 * @return     Error flag
 */
uint32_t cmd_switch_app(uint32_t app_id, uint32_t para2)
{
  if(app_id < APP_NUM) //check if id is valid
  {
    app_cfg_struct_init(app_id);
  }
  else
  {
    printf("?\r\n");
    return (uint32_t)-1;
  }
  if(g_p_app_curr) //pointer to current app object
    g_p_app_curr->pAppCtrl(APP_CTRL_STOP_NOW, 0); //pointer to current app object

  g_switch_app = 1;
  g_app_id = app_id;
  return 0;
}
