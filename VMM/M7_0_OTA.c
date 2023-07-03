 /************************Copyright by ofilm 2020-2030*****************************
  * @file    M7_0_OTA.c
  * @author  Application Team
  * @version V1.0.0
  * @date    1-6-2023
  * @brief   OTA manager
  *******************************************************************************/
#include "M7_0_OTA.h"
#include "Com_Cfg.h"

void M7_0_OTA_FiniteStateMachine(void);

typedef struct
{
    uint8 PwrSt;
    uint8 OTAPwrMngt;
    uint8 OTASts;
}M7_0_OTA_INPUT_VALUE;

typedef struct
{
    uint8 OTAPwrOnReqFb;
    uint8 OTAPwrDwnReqFb;
    uint8 OTAModStsFb;
}M7_0_OTA_OUTPUT_VALUE;


static M7_0_OTA_INPUT_VALUE s_OTAInputValue;
static M7_0_OTA_OUTPUT_VALUE s_OTAOutputValue;

static uint32 s_TickReceiveOTATimeout = 0;
static uint32 s_TickOTAUpgradeTimeout = 0;

static void OTAControlTickAdd(void)
{
    if (s_TickReceiveOTATimeout < UINT32_MAX)
    {
        s_TickReceiveOTATimeout += TASK_CYCLE_PERIOD;
    }

    if (s_TickOTAUpgradeTimeout < UINT32_MAX)
    {
        s_TickOTAUpgradeTimeout += TASK_CYCLE_PERIOD;
    }
}

static void ResetReceiveOTAStsTick(void)
{
    s_TickReceiveOTATimeout = 0;
}

static uint32 GetReceiveOTAStsTick(void)
{
    return s_TickReceiveOTATimeout;
}

static void ResetOTAUpgradeTick(void)
{
    s_TickOTAUpgradeTimeout = 0;
}

static uint32 GetOTAUpgradeTick(void)
{
    return s_TickOTAUpgradeTimeout;
}

static void M7_0_UpdateOTAInputValue(void)
{
    s_OTAInputValue.PwrSt = PowMode;
    s_OTAInputValue.OTAPwrMngt = IHUOTAPwrMngt;
    s_OTAInputValue.OTASts = IHUOTASts;
}

static void OTAStsSendFeedback(uint8 STATUS)
{
    b_Signal_BCAN_2EA_Tx_BDM_OTAModStsFb(STATUS);
    b_Signal_HYCAN_2EA_Tx_BDM_OTAModStsFb(STATUS);
    b_Signal_SUPPCAN_2EA_Tx_BDM_OTAModStsFb(STATUS);
    b_Signal_IFTCAN_2EA_Tx_BDM_OTAModStsFb(STATUS);
    b_Signal_ADASCAN_2EA_Tx_BDM_OTAModStsFb(STATUS);
    b_Signal_PWTCAN_2EA_Tx_BDM_OTAModStsFb(STATUS);
    b_Signal_CHSCAN_2EA_Tx_BDM_OTAModStsFb(STATUS);

    OTAModStsFb = STATUS;
}

static uint8 CheckOTAStsReceive(void)
{
    uint8 Flag = FALSE;
    if (COM_Get_RxMsgIndicationFlag(COM_RxMsg_IFTCAN_40B) == TRUE)
    {
        Flag = TRUE;
        ResetReceiveOTAStsTick();
        COM_Clr_RxMsgIndicationFlag(COM_RxMsg_IFTCAN_40B);
    }

    return Flag;
}

static M7_0_OTA_MODE s_OTA_FSM = MODE_NORMAL;
void M7_0_OTA_FiniteStateMachine(void)
{
    switch (s_OTA_FSM)
    {
        case MODE_NORMAL:
        {
            if ((s_OTAInputValue.PwrSt == PM_ON)&&((s_OTAInputValue.OTASts == OTA_STATUS)&&(CheckOTAStsReceive() == TRUE)))
            {
                /* Mode Switch */
                s_OTA_FSM = MODE_OTA;
                ResetOTAUpgradeTick();
                /* Send feedback signal */
                s_OTAOutputValue.OTAModStsFb = OTA_STS_ACTIVE;
                OTAStsSendFeedback(s_OTAOutputValue.OTAModStsFb);
            }
            break;
        }

        case MODE_OTA:
        {
            if ((s_OTAInputValue.OTASts == NORMAL_STATUS)&&(CheckOTAStsReceive() == TRUE))
            {
                /* Mode Switch */
                s_OTA_FSM = MODE_NORMAL;
                /* Send feedback signal */
                s_OTAOutputValue.OTAModStsFb = OTA_STS_INACTIVE;
                OTAStsSendFeedback(s_OTAOutputValue.OTAModStsFb);
            }

            if (GetReceiveOTAStsTick() > TIME_LMT_20MIN)
            {
                /* Mode Switch */
                s_OTA_FSM = MODE_NORMAL;
                /* Send feedback signal */
                s_OTAOutputValue.OTAModStsFb = OTA_STS_INACTIVE;
                OTAStsSendFeedback(s_OTAOutputValue.OTAModStsFb);
            }

            /* upgrade time exceeds 2 hours */
            if ((GetReceiveOTAStsTick() <= TIME_LMT_20MIN)&&(GetOTAUpgradeTick() > TIME_LMT_2HOUR))
            {
               /* Mode Switch */
                s_OTA_FSM = MODE_NORMAL;
                /* Send feedback signal */
                s_OTAOutputValue.OTAModStsFb = OTA_STS_INACTIVE;
                OTAStsSendFeedback(s_OTAOutputValue.OTAModStsFb); 
            }
            break;
        }

        default:
            break;
    }
}

void M7_0_OTA_Mode_Init(void)
{
    memset(&s_OTAInputValue, 0, sizeof(M7_0_OTA_INPUT_VALUE));
    memset(&s_OTAOutputValue, 0, sizeof(M7_0_OTA_OUTPUT_VALUE));
    M7_0_UpdateOTAInputValue();
}

void M7_0_OTAModeManagerRunnable(void)
{
    M7_0_UpdateOTAInputValue();
    OTAControlTickAdd();
    M7_0_OTA_FiniteStateMachine();
}

uint8 M7_0_OTA_Mode_Get(void)
{
    return s_OTA_FSM;
}

