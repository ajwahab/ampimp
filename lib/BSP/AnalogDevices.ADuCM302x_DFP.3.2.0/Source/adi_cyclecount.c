/*
 *******************************************************************************
 * @file:    cyclecount_logging.c
 *
 * @brief:   Framework to preform cycle count measurements
 *
 * @details  this is a framework for monitoring the cycle counts, specifically
 *           for ISRs and low level function.

*******************************************************************************

 Copyright(c) 2016 Analog Devices, Inc. All Rights Reserved.

 This software is proprietary and confidential.  By using this software you agree
 to the terms of the associated Analog Devices License Agreement.

 ******************************************************************************/

#include <adi_cyclecount.h>
#include <common.h>
#include <adi_processor.h>
#include <rtos_map/adi_rtos_map.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>


 /** @addtogroup  cyclecount_logging Cycle Counting Framework
 *   @{
 *  @brief  A framework for monitoring the adjusted and unadjusted cycle counts for ISRs and for APIs.
 *  @details <b>To enable cycle counting</b>
 *  @details     1) Copy the file <em>Include/config/adi_cycle_counting_config.h</em> to your project directory.
 *  @details     2) Set the macro #ADI_CYCLECOUNT_ENABLED to '1' in the project's copy of <em> Include/config/adi_cycle_counting_config.h.</em>
 *  @details     3) Enable ISRs and/or APIs for which cycle counts are to be obtained. The configuration macros that control this are also located in <em>adi_cycle_counting_config.h.</em>
 *  @details <b>Obtaining cycle counts</b>
 *  @details     1) Call #ADI_CYCLECOUNT_INITIALIZE() once at the start of the application
 *  @details     2) Call #adi_cyclecount_start() at the start of each code block (ISR or API) for which cycles counts are to be obtained.
 *  @details        This will 'push' the cycle counting context stack.
 *  @details     3) Call #ADI_CYCLECOUNT_STORE(id) at the end of each ISR/API. This will read and record the cycle count for the specified ID.
 *  @details     4) Call #adi_cyclecount_stop() also at the end of each ISR/API. This will pop the cycle counting context stack.
 *  @details     5) Call #ADI_CYCLECOUNT_REPORT() to print the cycle counts for enabled ISR/API
 *  @details     \n
 *  @details
 *  @details  <b> Adjusted versus Unadjusted cycle counts  </b> \n
 *  @details  Adjusted cycle counts are cycle counts from which any inner nested cycle counting has been subtracted.
 *  @details  Unadjusted cycle counts represent the total elapsed cycle count time including any nested cycle counts that are reported for nested ISRs or nested APIs.
 *  @details  For example, if function foo() is being cycle counted, and foo() is interrupted, the ISR cycle
 *  @details  counts are not included in the adjusted cycle count but they are included in the unadjusted cycle count.
 *  @details
 *
 *  @note The application must include adi_cyclecount.h to use this framework.
 *  @note This framework uses Systick for cycle counting. If an application uses Systick then the logic in the Systick ISR contained in this framework would need to be combined with the application's systick logic.
 *
 *  @note There is no need to call adi_cyclecount_start() and adi_cyclecount_stop() inside of an ISR as long as the ISR uses the ISR_PROLOG() and ISR_EPILOG() macros. The ISR_PROLOG and ISR_EPILOG macros definitions expand into code sequences that include the calls to adi_cyclecount_start() and adi_cyclecount_stop().
 *  @note Non-ISR code will, however, have to add calls to adi_cyclecount_start() and adi_cyclecount_stop() manually at the beginning and end of the function.
 *  @note
 */



#ifdef __ICCARM__

/*
 * IAR MISRA C 2004 error suppressions.
 *
 * Pm073 (rule 14.7): a function should have a single point of exit
 * Pm143 (rule 14.7): a function should have a single point of exit at the end of the function
 *   Multiple returns are used for error handling.
 * Pm023 (rule 9.2):  Braces shall be used to indicate and match the structure in the non-zero initialization of arrays and structures.
 *   Braces cannot be used to indicate and match the structure in the non-zero initialization of arrays because the total size of the array is defined by a configuration macro.
 *
 */
#pragma diag_suppress=Pm143,Pm073,Pm023
#endif  /* __ICCARM__ */

/*!
 *
 * Total cycle counting 'stack' requirements. It is the sum of the total number of predefined cycle count API/ISR IDs
 * plus the number of user define API/ISR IDs.
 *
 */
#define ADI_CYCLECOUNT_TOTAL_ID_SZ  (ADI_CYCLECOUNT_ID_COUNT + ADI_CYCLECOUNT_NUMBER_USER_DEFINED_APIS)

/*
 * Names of items for which cycle counting can be enabled.
 */
static const char * adi_cyclecounting_identifiers[ADI_CYCLECOUNT_TOTAL_ID_SZ] = {
  "",
  "ISR_EXT_3",
  "ISR_UART",
  "ISR_DMA_UART_TX",
  "ISR_DMA_UART_RX",
  "ISR_TMR_COMMON",
  "ISR_RTC",
  "ISR_SPI",
  "ISR_CRC",
  "ISR_SPORT"
};

static uint32_t adi_cyclecounting_identifiers_index = ADI_CYCLECOUNT_ID_COUNT;

/*
 * Data structure to keep track of the per item cycle counts
 */
static ADI_CYCLECOUNT_LOG cycleCounts[ADI_CYCLECOUNT_TOTAL_ID_SZ];

/*!
 * Global variable used to track how many Systick interrupts have fired
 */
static volatile uint32_t nNumSystickInterrupts = 0u;

#if ADI_CYCLECOUNT_ENABLED == 1u
void SysTick_Handler(void);

/*!
 * Systick handler. Overrides the weak definition. To avoid replacing the weak
 * definition or other definitions from other modules when cycle counting is
 * disabled, this definition is under the #ADI_CYCLECOUNT_ENABLED macro
 */
void SysTick_Handler(void)
{
  ISR_PROLOG()
  nNumSystickInterrupts++; /* Bump the number of times systick has fired an interrupt */
  ISR_EPILOG()
}
#endif

/*!
 * In order to compute the number of cycles spent in a given item care needs to
 * be taken to subtract out any nested cycles that might occur due to nesting
 * interrupts.
 *
 * For example, if the SPI ISR fires but it is interrupted by the Systick ISR
 * then the cycle counting code has to subtract out of the SPI ISR cycle count
 * the number of cycles spent in the Systick ISR
 *
 *
 * The following structure maintains a call stack of sorts. Every time a new
 * ISR is entered it automatically will "push" a new cycle counting context
 * onto the cycle counting "stack". When an ISR exits it will "pop" the cycle
 * counting stack.
 *
 * See also the APIs adi_cyclecount_start() and adi_cyclecount_stop()
 *
 */
static struct
{
  adi_cyclecount_t starting_cycle_count;             /*!< The adjusted cycle count */
  adi_cyclecount_t adjusted_cycle_count;             /*!< The unadjusted cycle count */
} cycle_count_stack[ADI_CYCLECOUNT_STACK_SIZE];

/* The "top of the cycle counting stack". Used to index into the cycle_count_stack[] array */
static int32_t cycle_count_stack_ndx = ADI_CYCLECOUNT_INITIAL_STACK_INDEX;

/*!
 * @brief     Read the current number of cycle counts
 *
 * @details   Return the Cycle Counts which are recorded using Systick
 *
 * @return    adi_cyclecount_t
 *
*/
adi_cyclecount_t adi_cyclecount_get(void)
{
  ADI_INT_STATUS_ALLOC();
  ADI_ENTER_CRITICAL_REGION();

  adi_cyclecount_t systickValue = (uint64_t)ADI_CYCLECOUNT_SYSTICKS - SysTick->VAL; /* down counter */
  systickValue = systickValue + ((uint64_t)ADI_CYCLECOUNT_SYSTICKS * nNumSystickInterrupts);

  ADI_EXIT_CRITICAL_REGION();
  return systickValue;
}

/*!
 * @brief     API to be called to start a new cycle counting context.
 *
 * @details   Every entity (ISR or API) for which cycle counts are needed
 *            must call this routine at the beginning of the code sequence
 *            for which a cycle count is needed.
 *
 * @return    ADI_CYCLECOUNT_RESULT
 *            -# ADI_CYCLECOUNT_SUCCESS    Function successfully pushed the cycle context
 *            -# ADI_CYCLECOUNT_FAILURE    Error. Cycle Counting stack overflow.
 *
 * @note If adi_cyclecount_start() returns false then the cycle count stack size should
 * @note be increased. The macro #ADI_CYCLECOUNT_INITIAL_STACK_INDEX controls the stack
 * @note size.
 *
*/

ADI_CYCLECOUNT_RESULT adi_cyclecount_start(void)
{
  if( (ADI_CYCLECOUNT_STACK_SIZE-1) == cycle_count_stack_ndx)
  {
    return ADI_CYCLECOUNT_FAILURE; /* need to increase the size of the nesting stack ADI_CYCLECOUNT_STACK_SIZE. */
  }
  cycle_count_stack_ndx++;

  cycle_count_stack[cycle_count_stack_ndx].starting_cycle_count = adi_cyclecount_get();
  cycle_count_stack[cycle_count_stack_ndx].adjusted_cycle_count = 0u;

  return ADI_CYCLECOUNT_SUCCESS;
}

/*!
 * @brief      Cycle Count Stop
 *
 * @details    This function ends an ISR or API context. It should be called
 *             at the end of the code sequence for which cycle counting is
 *             enabled.
 *
 * @return     Success
 *             -# ADI_CYCLECOUNT_SUCCESS    No errors
 *             -# ADI_CYCLECOUNT_FAILURE   There is a adi_cyclecount_start/adi_cyclecount_stop mismatch
 *
 */



ADI_CYCLECOUNT_RESULT adi_cyclecount_stop(void)
{
  if( cycle_count_stack_ndx == ADI_CYCLECOUNT_INITIAL_STACK_INDEX )
  {
    /* start/stop mismatch. Every call to adi_cyclecount_start() should be paired with */
    /* a call to adi_cyclecount_stop().                                                 */
    return ADI_CYCLECOUNT_FAILURE;
  }
  /* Check for nesting. If we are stacked up on top of another item's cycle count then */
  /* record the time spent at this cycle counting nesting level. This will allow the   */
  /* nested context to subtract this number of cycles.                                 */
  if( cycle_count_stack_ndx > 0 )
  {

    /* stacked cycle counts probably caused by an ISR */
    /* adjust by adding in the number of cycles taken by the interrupting code */
    adi_cyclecount_t currentTime = adi_cyclecount_get();
    adi_cyclecount_t elapsedTime = currentTime -
                                   cycle_count_stack[cycle_count_stack_ndx].starting_cycle_count -
                                   cycle_count_stack[cycle_count_stack_ndx].adjusted_cycle_count;

    /* compute the number of cycles that the interrupting code consumed and store this */
    /* number at next cycle counting stack level                                        */
    cycle_count_stack[cycle_count_stack_ndx-1].adjusted_cycle_count += elapsedTime;

  }

  cycle_count_stack_ndx--;
  return ADI_CYCLECOUNT_SUCCESS;
}

/*!
 * @brief     API to be called to initialize the cycle counting framework.
 *
 * @details   The cycle counting framework uses systick. Sysyick is, therefore,
 *            initialized. If an application uses Systick then the logic in the Systick ISR
 *            contained in this framework would need to be combined with the application's systick logic.
 *
 *
*/
void adi_cyclecount_init(void){
  uint32_t i;

  /* Configure SysTick */
  SysTick_Config(ADI_CYCLECOUNT_SYSTICKS);

  /* initialize log */
  for(i=0u; i<ADI_CYCLECOUNT_TOTAL_ID_SZ; i++){
    cycleCounts[i].min_cycles_adjusted = UINT64_MAX;
    cycleCounts[i].max_cycles_adjusted = 0u;
    cycleCounts[i].average_cycles_adjusted = 0u;
    cycleCounts[i].min_cycles_unadjusted = UINT64_MAX;
    cycleCounts[i].max_cycles_unadjusted = 0u;
    cycleCounts[i].average_cycles_unadjusted = 0u;
    cycleCounts[i].sample_count = 0u;
  }

  return;
}

/*!
 * @brief     Add an ISR/API to the cycle counting list.
 *
 * @details   Add the application API/ISR entity to the list of API/ISRs for which cycle counts
 *            can be obtained. The function will first check to make sure there is enough space
 *            in the list. If there is the list ID will be written. If there is not enough
 *            space then an error will be returned (and the application would need to increase
 *            the size of the list by modifying the macro ADI_CYCLECOUNT_NUMBER_USER_DEFINED_APIS.
 *
 * @param [in]  EntityName  Name of the API/ISR for which cycle counts are to be obtained.
 * @param [out] pid         Pointer to the ID for this entity.
 *
 * @return    ADI_CYCLECOUNT_RESULT
 *            -# ADI_CYCLECOUNT_SUCCESS               Function successfully pushed the cycle context
 *            -# ADI_CYCLECOUNT_ADD_ENTITY_FAILURE    Not enough room in the cycle counting enity stack.
 *
*/
ADI_CYCLECOUNT_RESULT adi_cyclecount_addEntity(const char *EntityName, uint32_t *pid)
{

  if( adi_cyclecounting_identifiers_index >= (ADI_CYCLECOUNT_TOTAL_ID_SZ))
  {
    return ADI_CYCLECOUNT_ADD_ENTITY_FAILURE;
  }

  adi_cyclecounting_identifiers[adi_cyclecounting_identifiers_index] = EntityName;
  *pid = adi_cyclecounting_identifiers_index;
  adi_cyclecounting_identifiers_index++;

  return ADI_CYCLECOUNT_SUCCESS;
}


/*!
 * @brief     Store Cycle Count
 *
 * @details   adi_cyclecount_store will calculate the number of cycles
 *            since the last call to adi_cyclecount_start().
 *            The cycle count is then recorded against the API/ISR identified by 'id'
 *
 * @param [in] id     Specifies the API/ISR that this data belongs to
 *
 * @return    ADI_CYCLECOUNT_RESULT
 *            -# ADI_CYCLECOUNT_SUCCESS               Function successfully stored the cycle count
 *            -# ADI_CYCLECOUNT_ADD_ENTITY_FAILURE    Not enough room in the cycle counting enity stack.
 */
ADI_CYCLECOUNT_RESULT adi_cyclecount_store(uint32_t id){

#ifdef ADI_DEBUG
  if( (id == 0u) || (id >= (ADI_CYCLECOUNT_TOTAL_ID_SZ)))
  {
    return ADI_CYCLECOUNT_INVALID_ID;
  }
  if( (cycle_count_stack_ndx < 0) || ((cycle_count_stack_ndx >= ADI_CYCLECOUNT_STACK_SIZE)))
  {
    return ADI_CYCLECOUNT_FAILURE;
  }

#endif

  adi_cyclecount_t systickValue = adi_cyclecount_get();

  cycleCounts[id].sample_count++;

  adi_cyclecount_t startCycleCount = cycle_count_stack[cycle_count_stack_ndx].starting_cycle_count;
  adi_cyclecount_t adjustCycles = cycle_count_stack[cycle_count_stack_ndx].adjusted_cycle_count;
  adjustCycles += startCycleCount;

  adi_cyclecount_t adjustedValue = systickValue - adjustCycles;

  /****************************************************
   * Update the adjusted cycle counts
   ***************************************************/
  if(adjustedValue > cycleCounts[id].max_cycles_adjusted ){
    cycleCounts[id].max_cycles_adjusted = adjustedValue;
  }
  if(adjustedValue < cycleCounts[id].min_cycles_adjusted){
    cycleCounts[id].min_cycles_adjusted = adjustedValue;
  }
  /* Accumulate the total cycle count from which the average will be calcuated  */
  cycleCounts[id].average_cycles_adjusted += adjustedValue;

  /****************************************************
   * Update the unadjusted cycle counts
   ****************************************************/
  adi_cyclecount_t unadjustedCycleCount = systickValue - startCycleCount;
  if(unadjustedCycleCount > cycleCounts[id].max_cycles_unadjusted ){
    cycleCounts[id].max_cycles_unadjusted = unadjustedCycleCount;
  }
  if(unadjustedCycleCount < cycleCounts[id].min_cycles_unadjusted){
    cycleCounts[id].min_cycles_unadjusted = unadjustedCycleCount;
  }
  /* Accumulate the total cycle count from which the average will be calcuated  */
  cycleCounts[id].average_cycles_unadjusted += unadjustedCycleCount;

  return ADI_CYCLECOUNT_SUCCESS;
}


/*!
 * @brief     Generate a cycle count report
 *
 * @details   The cycle count data will be printed.
 *            Output will be sent to either a console I/O window
 *            or to the UART, depending on how DEBUG_MESSAGE is configured.
 *
 * @note      It is not valid to call any other cycle counting API while
 *            this API is functioning
 *
*/
void adi_cyclecount_report(void){
  adi_cyclecount_t i;

  DEBUG_MESSAGE("                                         Cycle Count Report\n");
  DEBUG_MESSAGE("                          Adjusted Cycle Counts          Unadjusted Cycles Counts         No. Samples\n");
  DEBUG_MESSAGE("                     _____________________________      ____________________________      ___________\n");
  DEBUG_MESSAGE("ISR/API              Minimum    Maximum    Average      Minimum    Maximum    Average\n");

  for(i=1u; i<(uint32_t)ADI_CYCLECOUNT_TOTAL_ID_SZ; i++){

    if( cycleCounts[i].sample_count )
    {

      DEBUG_MESSAGE(
            "%15s    %10llu %10llu %10llu   %10llu %10llu %10llu %10lu\n",
            adi_cyclecounting_identifiers[i],
            cycleCounts[i].min_cycles_adjusted,
            cycleCounts[i].max_cycles_adjusted,
            cycleCounts[i].average_cycles_adjusted/cycleCounts[i].sample_count,
            cycleCounts[i].min_cycles_unadjusted,
            cycleCounts[i].max_cycles_unadjusted,
            cycleCounts[i].average_cycles_unadjusted/cycleCounts[i].sample_count,
            cycleCounts[i].sample_count
           );
    }
  }

  return;
}


/* @} */
