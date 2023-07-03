 /************************Copyright by ofilm 2020-2030*****************************
  * @file    M7_0_SOC.c
  * @author  Application Team
  * @version V1.0.0
  * @date    30-5-2023
  * @brief   SOC manager
  *******************************************************************************/
#include "M7_0_SOC.h"
#include "M7_0_VMM.h"
#include "NM.h"

// void M7_0_PowerModeOutput(void);
void M7_0_LimitLevelJudge(void);
void M7_0_LowBatteryWarning(void);
void M7_0_BatterStateInquiry(void);

typedef struct 
{
    uint8 SOC;
    uint8 SOH;
    uint16 I_BATT;
    uint16 U_BATT;
    uint8 T_BATT;
    uint8 I_RANGE;
    uint8 SOF_V1;
    uint8 SOC_STATE;
}M7_0_INPUT_VALUE;

typedef struct 
{
    uint8 VehPwrMod;
    uint8 IBS_SOC;
    uint16 IBS_Current;
    uint8 IBS_CurrentRange;
    uint8 Lbatip;
    uint16 IBS_Voltage;
    uint8 IBS_BatteryTemp;
    uint8 IBS_STAT_SOC;
    uint8 IBS_SOF_V1;
    uint8 IBS_SOH;
    uint8 LbaLimits;
}M7_0_OUTPUT_VALUE;

static M7_0_INPUT_VALUE s_SOCInputValue;
static M7_0_OUTPUT_VALUE s_SOCOutputValue;
static uint8 s_PowerState = PM_OFF;
static uint8 s_NetworkState = NETWORK_WAKE_UP;

/* Can Tx global var */
uint8 g_u8VMM_LbaLimits = 0;
uint8 g_u8VMM_Lbatip = 0;
uint16 g_u16VMM_IBS_Current = 0;
uint16 g_u16VMM_IBS_Voltage = 0;
uint8 g_u8VMM_IBS_BatteryTemp = 0;
uint8 g_u8VMM_IBS_SOC = 0;
uint8 g_u8VMM_IBS_SOH = 0;
uint8 g_u8VMM_IBS_STAT_SOC = 0;
uint8 g_u8VMM_IBS_SOF_V1 = 0;
uint8 g_u8VMM_IBS_CurrentRange = 0;

/* Lin Rx global var */
uint8 g_u8VMM_SOC = 0;
uint8 g_u8VMM_SOH = 0;
uint8 g_u16VMM_I_BATT = 0;
uint8 g_u16VMM_U_BATT = 0;
uint8 g_u8VMM_I_RANGE = 0;
uint8 g_u8VMM_SOF_V1 = 0;
uint8 g_u8VMM_SOC_STATE = 0;


#define MAX_FILTER_NUM 10
typedef struct 
{
    uint16 FilterBuff[MAX_FILTER_NUM];
    uint16 AverageValue;
    uint8 Index;
    uint8 FlagArrayFull;
}M7_0_I_BATT_FILTER;

static M7_0_I_BATT_FILTER s_IBATT;

static void M7_0_UpdateSOCInputValue(void)
{
    s_PowerState = PowMode;
    s_NetworkState = NM_GetNmSleepState(); /* 1-sleep 0-wake up */

    s_SOCInputValue.SOC = g_u8VMM_SOC;
    s_SOCInputValue.SOH = g_u8VMM_SOH;
    s_SOCInputValue.I_BATT = g_u16VMM_I_BATT;
    s_SOCInputValue.U_BATT = g_u16VMM_U_BATT;
    s_SOCInputValue.I_RANGE = g_u8VMM_I_RANGE;
    s_SOCInputValue.SOF_V1 = g_u8VMM_SOF_V1;
    s_SOCInputValue.SOC_STATE = g_u8VMM_SOC_STATE;

    /* I_BATT value filter */
    if (s_IBATT.Index < MAX_FILTER_NUM)
    {
        s_IBATT.FilterBuff[s_IBATT.Index] = s_SOCInputValue.I_BATT;
        s_IBATT.Index++;
    }
    else
    {
        s_IBATT.FlagArrayFull = TRUE;
        s_IBATT.Index = 0;
        s_IBATT.FilterBuff[s_IBATT.Index] = s_SOCInputValue.I_BATT;
        s_IBATT.Index++;
    }

    uint32 sum = 0;
    if (s_IBATT.FlagArrayFull != TRUE)
    {
        for(uint8 i = 0; i <= s_IBATT.Index; i++)
        {
            sum += s_IBATT.FilterBuff[i];
        }
        s_IBATT.AverageValue = sum / (s_IBATT.Index + 1);
    }
    else
    {
        for(uint8 i = 0; i < MAX_FILTER_NUM; i++)
        {
            sum += s_IBATT.FilterBuff[i];
        }
        s_IBATT.AverageValue = sum / MAX_FILTER_NUM;
    }
}

static float64 M7_0_CurrentCalculate(uint8 I_RANGE, uint16 I_BATT)
{
    float64 temp = 0.0;
    switch (I_RANGE)
    {
        case RANGE_0:
        {
            if ((I_BATT >= MINIMUM_RANGE_0)&&(I_BATT <= MAXIMUM_RANGE_0))
            {
                temp = ((float64)I_BATT * FACTOR_RANGE_0 + OFFSET_RANGE_0);
            }
            else
            {
                temp = 0xFFFF;
            }
            break;
        }
        case RANGE_1:
        {
            if ((I_BATT >= MINIMUM_RANGE_1)&&(I_BATT <= MAXIMUM_RANGE_1))
            {
                temp = ((float64)I_BATT * FACTOR_RANGE_1 + OFFSET_RANGE_1);
            }
            else
            {
                temp = 0xFFFF;
            }
            break;
        }
        case RANGE_2:
        {
            if ((I_BATT >= MINIMUM_RANGE_1)&&(I_BATT <= MAXIMUM_RANGE_1))
            {
                temp = ((float64)I_BATT * FACTOR_RANGE_2 + OFFSET_RANGE_2);
            }
            else
            {
                temp = 0xFFFF;
            }
            break;
        }
        case RANGE_INVALID:
        {
            temp = 0xFFFF;
            break;
        }
        default:
            break;
    }
    return temp;
}

// void M7_0_PowerModeOutput(void)
// {
//     if (s_NetworkState != NETWORK_WAKE_UP)
//     {
//         return;
//     }

//     // upload to the network
//     g_u8VehPwrMod = s_PowerState;
// }

void M7_0_LimitLevelJudge(void)
{
    uint8 PwrSt = s_PowerState; /* get power state */
    /* Power State must be one of OFF/ACC/ON/RUNNING */
    if ((PwrSt != PM_OFF)&&(PwrSt != PM_ACC)&&(PwrSt != PM_ON))
    {
        return;
    }

    uint8 LimitLevel = s_SOCOutputValue.LbaLimits;
    float64 I_Value = M7_0_CurrentCalculate(s_SOCInputValue.I_RANGE, s_IBATT.AverageValue);

    switch (LimitLevel)
    {
        case LMT_LEVEL0:
        {
            if ((s_SOCInputValue.SOC <= 55)&&(s_SOCInputValue.SOC > 50)&&(I_Value < 0.0))
            {
                LimitLevel = LMT_LEVEL1;
            }
            break;
        }
        case LMT_LEVEL1:
        {
            if (s_SOCInputValue.SOC >= 57)
            {
                LimitLevel = LMT_LEVEL0;
            }
            else if ((s_SOCInputValue.SOC <= 50)&&(s_SOCInputValue.SOC > 45)&&(I_Value < 0.0))
            {
                LimitLevel = LMT_LEVEL2;
            }
            break;
        }
        case LMT_LEVEL2:
        {
            if (s_SOCInputValue.SOC >= 52)
            {
                LimitLevel = LMT_LEVEL1;
            }
            else if ((s_SOCInputValue.SOC <= 45)&&(I_Value < 0.0))
            {
                LimitLevel = LMT_LEVEL3;
            }
            break;
        }
        case LMT_LEVEL3:
        {
            if (s_SOCInputValue.SOC >= 47)
            {
                LimitLevel = LMT_LEVEL2;
            }
            break;
        }
        default:
            break;
    } /* END OF SWITCH */

    /* update limit level */
    s_SOCOutputValue.LbaLimits = LimitLevel;
    g_u8VMM_LbaLimits = LimitLevel;
}

void M7_0_LowBatteryWarning(void)
{
    uint8 NetworkSt = s_NetworkState;
    uint8 VMMSt = M7_0_VMM_Mode_Get();
    uint8 PwrSt = s_PowerState;

    if ((s_SOCInputValue.SOC == 0)||(VMMSt == MODE_TYPE_INIT)||(NetworkSt == NETWORK_SLEEP))
    {
        return;
    }

    if (((PwrSt == PM_OFF)||(PwrSt == PM_ACC)||(PwrSt == PM_ON))&&(s_SOCOutputValue.LbaLimits == LMT_LEVEL0))
    {
        s_SOCOutputValue.Lbatip = NO_WARNING;
    }
    else if (((PwrSt == PM_OFF)||(PwrSt == PM_ACC)||(PwrSt == PM_ON))&&
             ((s_SOCOutputValue.LbaLimits == LMT_LEVEL1)||(s_SOCOutputValue.LbaLimits == LMT_LEVEL2)||(s_SOCOutputValue.LbaLimits == LMT_LEVEL3)))
    {
        s_SOCOutputValue.Lbatip = WARNING_1;
    }
    else if ((PwrSt == PM_ON)&&
             ((s_SOCOutputValue.LbaLimits == LMT_LEVEL1)||(s_SOCOutputValue.LbaLimits == LMT_LEVEL2)||(s_SOCOutputValue.LbaLimits == LMT_LEVEL3)))
    {
        s_SOCOutputValue.Lbatip = WARNING_3;
    }

    /* upload to network */
    g_u8VMM_Lbatip = s_SOCOutputValue.Lbatip;
}

void M7_0_BatterStateInquiry(void)
{
    uint8 PwrSt = s_PowerState;

    if ((PwrSt != PM_OFF)&&(PwrSt != PM_ACC)&&(PwrSt != PM_ON)&&(PwrSt != PM_START))
    {
        return;
    }

    s_SOCOutputValue.IBS_Current = s_SOCInputValue.I_BATT;
    s_SOCOutputValue.IBS_Voltage = s_SOCInputValue.U_BATT;
    s_SOCOutputValue.IBS_BatteryTemp = s_SOCInputValue.T_BATT;
    s_SOCOutputValue.IBS_SOC = s_SOCInputValue.SOC;
    s_SOCOutputValue.IBS_SOH = s_SOCInputValue.SOH;
    s_SOCOutputValue.IBS_STAT_SOC = s_SOCInputValue.SOC_STATE;
    s_SOCOutputValue.IBS_SOF_V1 = s_SOCInputValue.SOF_V1;
    s_SOCOutputValue.IBS_CurrentRange = s_SOCInputValue.I_RANGE;

    /* T-box send data */
    /* DMC/IHU send data */
    g_u16VMM_IBS_Current = s_SOCOutputValue.IBS_Current;
    g_u16VMM_IBS_Voltage = s_SOCOutputValue.IBS_Voltage;
    g_u8VMM_IBS_BatteryTemp = s_SOCOutputValue.IBS_BatteryTemp;
    g_u8VMM_IBS_SOC = s_SOCOutputValue.IBS_SOC;
    g_u8VMM_IBS_SOH = s_SOCOutputValue.IBS_SOH;
    g_u8VMM_IBS_STAT_SOC = s_SOCOutputValue.IBS_STAT_SOC;
    g_u8VMM_IBS_SOF_V1 = s_SOCOutputValue.IBS_SOF_V1;
    g_u8VMM_IBS_CurrentRange = s_SOCOutputValue.IBS_CurrentRange;
}

// void M7_0_SOC_Manager_Ban(void)
// {
//     uint8 PwrSt = s_PowerState;
//     uint8 CurHeaterSt = g_u16SENSOR_SteeringWheelHeatingIn; /* Steering Wheel heater */
//     uint8 NextHeaterSt = CurHeaterSt;
//     uint8 LimitLevel = s_SOCOutputValue.LbaLimits;

//     if (PwrSt != PM_ON)
//     {
//         return;
//     }

//     switch (LimitLevel)
//     {
//         /* Level0 and Level 1 no limit */
//         case LMT_LEVEL0:
//         case LMT_LEVEL1:
//         {
//             break;
//         }
//         /* level2 and level3 heater off */
//         case LMT_LEVEL2:
//         case LMT_LEVEL3:
//         {
//             NextHeaterSt = HEATER_OFF;
//             break;
//         }
//     }

//     if (CurHeaterSt != NextHeaterSt)
//     {
//         /* Control heater */
//         b_Signal_BCAN_391_Tx_SteeringWheelheatingSts(NextHeaterSt);
//         b_Signal_SUPPCAN_391_Tx_SteeringWheelheatingSts(NextHeaterSt);
//         b_Signal_IFTCAN_391_Tx_SteeringWheelheatingSts(NextHeaterSt);
//         b_Signal_ADASCAN_391_Tx_SteeringWheelheatingSts(NextHeaterSt);
//         b_Signal_PWTCAN_391_Tx_SteeringWheelheatingSts(NextHeaterSt);
//         b_Signal_CHSCAN_391_Tx_SteeringWheelheatingSts(NextHeaterSt);
//     }
// }

void M7_0_SOC_Manager_Init(void)
{
    memset(&s_SOCInputValue, 0, sizeof(M7_0_INPUT_VALUE));
    memset(&s_SOCOutputValue, 0, sizeof(M7_0_OUTPUT_VALUE));
    memset(&s_IBATT, 0, sizeof(M7_0_I_BATT_FILTER));
    M7_0_UpdateSOCInputValue();
}

void M7_0_SOCModeManagerRunnable(void)
{
    M7_0_UpdateSOCInputValue();
    // M7_0_PowerModeOutput();
    M7_0_LimitLevelJudge();
    M7_0_LowBatteryWarning();
    M7_0_BatterStateInquiry();
    // M7_0_SOC_Manager_Ban();
}

uint8 M7_0_SOC_Mode_Get(void)
{
    return s_SOCOutputValue.LbaLimits;
}

