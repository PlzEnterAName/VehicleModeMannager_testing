 /************************Copyright by ofilm 2020-2030*****************************
  * @file    M7_0_VMMPublic.h 
  * @author  Application Team
  * @version V1.0.0
  * @date    31-5-2023
  * @brief   Header VMM  module
  *******************************************************************************/
#ifndef __M7_0_PMM_PUBLIC_H_
#define __M7_0_PMM_PUBLIC_H_

#include <stdint.h>
#include <string.h>
#include "Rte_CM7_0_GlobalVariables.h"
#include "CM7_0_Sensor_SWC.h"

#include "M7_0_VMM.h"
#include "M7_0_SOC.h"
#include "M7_0_OTA.h"

#define VMM_CM7_0_VERSION           VMM_CM7_0_000007

typedef enum
{
    PM_OFF,
    PM_ACC,
    PM_ON,      /* IGN_ON, RUNNING */
    PM_START    /* CRANK */
}M7_0_POWER_MODE;

typedef enum
{
    ENG_NOT_RUNNING,
    ENG_RUNNING,
}M7_0_ENGINE_STATE;

typedef enum
{
    NETWORK_WAKE_UP = 0,
    NETWORK_SLEEP,
}M7_0_NETWORK_STATE;

typedef enum
{
    HEATER_OFF = 0,
    HEATER_ON,
}M7_0_HEATER_STATE;

typedef enum
{
    MASTER_LIGHT_OFF = 0,
    MASTER_LIGHT_AUTO,
    MASTER_LIGHT_POSITION_ON,
    MASTER_LIGHT_LOWBEAM_ON
}M7_0_MASTER_LIGHT_STATE;

#define TASK_CYCLE_PERIOD 50 /* unint : ms */

#define VMM_U32_MAX 0xFFFFFFFF
#define VMM_U16_MAX 0xFFFF
#define VMM_U8_MAX  0xFF

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* Can Tx global var */
extern uint8 g_u8VMM_LbaLimits;
extern uint8 g_u8VMM_Lbatip;
extern uint16 g_u16VMM_IBS_Current;
extern uint16 g_u16VMM_IBS_Voltage;
extern uint8 g_u8VMM_IBS_BatteryTemp;
extern uint8 g_u8VMM_IBS_SOC;
extern uint8 g_u8VMM_IBS_SOH;
extern uint8 g_u8VMM_IBS_STAT_SOC;
extern uint8 g_u8VMM_IBS_SOF_V1;
extern uint8 g_u8VMM_IBS_CurrentRange;
/* Lin Rx global var */
extern uint8 g_u8VMM_SOC;
extern uint8 g_u8VMM_SOH;
extern uint8 g_u16VMM_I_BATT;
extern uint8 g_u16VMM_U_BATT;
extern uint8 g_u8VMM_I_RANGE;
extern uint8 g_u8VMM_SOF_V1;
extern uint8 g_u8VMM_SOC_STATE;


uint8 M7_0_VMM_Mode_Get(void);
uint8 M7_0_SOC_Mode_Get(void);
uint8 M7_0_OTA_Mode_Get(void);

#endif
