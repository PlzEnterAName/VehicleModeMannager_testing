 /************************Copyright by ofilm 2020-2030*****************************
  * @file    M7_0_VMM.c
  * @author  Application Team
  * @version V1.0.0
  * @date    26-5-2023
  * @brief   vehicle mode manager
  *******************************************************************************/
#include "M7_0_VMM.h"

void Vehicle_Mode_SW_Handle(uint8 VehicleMode);
uint8 M7_0_VMM_CheckPowerState(void);
void M7_0_VMM_FiniteStateMachine(void);
void M7_0_FindKeyTriggerEvent(void);
uint8 UpdateVehicleMode(uint8 vehicle_mode);
void M7_0_VMM_DiagnosticHandle(void);

typedef struct 
{
    uint8 State;
    uint8 RequestMode;
    M7_0_ROUTINE_FAIL_REASON FailReason;
}M7_0_VMM_DIAG;

static M7_0_VMM_DIAG s_VMM_DiagInfo =
{
    .State = ROUTINE_STOPPED,
    .RequestMode = TEL_DIAG_NONE,
    .FailReason.value = 0,
};

typedef struct 
{
    float Speed;            
    uint8 PowerState;    
    uint8 EngineState;   
    uint16 KeyIn;         
    uint16 HazardSW;      
    uint16 HighBeamSW;    
    uint16 MasterLightSW; 
    uint16 CrashIn;       
    uint16 BrakeSW;       
}M7_0_VMM_CUR_VALUE;

static M7_0_VMM_CUR_VALUE s_VMMValue;


/************************* Finite State Machine *****************************/
typedef struct {
    uint8 event;
    uint8 CurState;
    void (*eventActFun)(uint8);
    uint8 NextState;
}StateTable;

typedef struct {
    uint8 curState;
    StateTable *stateTable;
    uint8 size;
}fsmType;

static fsmType s_Vehicle_FSM;

const StateTable Vehicle_FSM_Table[] = {
    /******** event ***************************** current mode **************** event handle ************ next mode ******************/
    { VMM_EVENT_FACTORY_TO_USER,                MODE_TYPE_FACTORY,           Vehicle_Mode_SW_Handle,     MODE_TYPE_USER               },
    { VMM_EVENT_FACTORY_TO_USER,                MODE_TYPE_FACTORY_PAUSED,    Vehicle_Mode_SW_Handle,     MODE_TYPE_USER               },
    { VMM_EVENT_TRANSPORT_TO_USER,              MODE_TYPE_TRANSPORT,         Vehicle_Mode_SW_Handle,     MODE_TYPE_USER               },
    { VMM_EVENT_DYNO_TO_USER,                   MODE_TYPE_DYNO,              Vehicle_Mode_SW_Handle,     MODE_TYPE_USER               },
    { VMM_EVENT_CRASH_TO_USER,                  MODE_TYPE_CRASH,             Vehicle_Mode_SW_Handle,     MODE_TYPE_USER               },
    { VMM_EVENT_CRASH_TO_FACTORY_PAUSED,        MODE_TYPE_CRASH,             Vehicle_Mode_SW_Handle,     MODE_TYPE_FACTORY_PAUSED     },
    { VMM_EVENT_CRASH_TO_TRANSPORT,             MODE_TYPE_CRASH,             Vehicle_Mode_SW_Handle,     MODE_TYPE_TRANSPORT          },
    { VMM_EVENT_FACTORY_TO_FACTORY_PAUSED,      MODE_TYPE_FACTORY,           Vehicle_Mode_SW_Handle,     MODE_TYPE_FACTORY_PAUSED     },
    { VMM_EVENT_FACTORY_PAUSED_TO_FACTORY,      MODE_TYPE_FACTORY_PAUSED,    Vehicle_Mode_SW_Handle,     MODE_TYPE_FACTORY            },
    { VMM_EVENT_FACTORY_PAUSED_TO_TRANSPORT,    MODE_TYPE_FACTORY_PAUSED,    Vehicle_Mode_SW_Handle,     MODE_TYPE_TRANSPORT          },
    { VMM_EVENT_FACTORY_PAUSED_TO_USER,         MODE_TYPE_FACTORY_PAUSED,    Vehicle_Mode_SW_Handle,     MODE_TYPE_USER               },
    { VMM_EVENT_TRANSPORT_TO_TRANPORT_PAUSED,   MODE_TYPE_TRANSPORT,         Vehicle_Mode_SW_Handle,     MODE_TYPE_TRANSPORT_PAUSED   },
    { VMM_EVENT_TRANSPORT_TO_FACTORY_PAUSED,    MODE_TYPE_TRANSPORT,         Vehicle_Mode_SW_Handle,     MODE_TYPE_FACTORY_PAUSED     },
    { VMM_EVENT_TRANSPORT_PAUSED_TO_TRANPORT,   MODE_TYPE_TRANSPORT_PAUSED,  Vehicle_Mode_SW_Handle,     MODE_TYPE_TRANSPORT          },
    { VMM_EVENT_USER_TO_DYNO,                   MODE_TYPE_USER,              Vehicle_Mode_SW_Handle,     MODE_TYPE_DYNO               },
    { VMM_EVENT_USER_TO_FACTORY_PAUSED,         MODE_TYPE_USER,              Vehicle_Mode_SW_Handle,     MODE_TYPE_FACTORY_PAUSED     },
    { VMM_EVENT_USER_TO_TRANSPORT,              MODE_TYPE_USER,              Vehicle_Mode_SW_Handle,     MODE_TYPE_TRANSPORT          },
    { VMM_EVENT_CRASH,                          MODE_TYPE_USER,              Vehicle_Mode_SW_Handle,     MODE_TYPE_CRASH              },
    { VMM_EVENT_CRASH,                          MODE_TYPE_FACTORY,           Vehicle_Mode_SW_Handle,     MODE_TYPE_CRASH              },
    { VMM_EVENT_CRASH,                          MODE_TYPE_FACTORY_PAUSED,    Vehicle_Mode_SW_Handle,     MODE_TYPE_CRASH              },
    { VMM_EVENT_CRASH,                          MODE_TYPE_TRANSPORT,         Vehicle_Mode_SW_Handle,     MODE_TYPE_CRASH              },
    { VMM_EVENT_CRASH,                          MODE_TYPE_TRANSPORT_PAUSED,  Vehicle_Mode_SW_Handle,     MODE_TYPE_CRASH              },
};

void Vehicle_Mode_SW_Handle(uint8 VehicleMode)
{
    switch (VehicleMode)
    {
        case MODE_TYPE_FACTORY:
        {
            SlaveVehMod = MODE_TYPE_FACTORY;
            MstVehMod = MODE_TYPE_FACTORY;
            break;
        }
        case MODE_TYPE_TRANSPORT:
        {
            SlaveVehMod = MODE_TYPE_TRANSPORT;
            MstVehMod = MODE_TYPE_TRANSPORT;
            break;
        }
        case MODE_TYPE_USER:
        {
            SlaveVehMod = MODE_TYPE_USER;
            MstVehMod = MODE_TYPE_USER;
            break;
        }
        case MODE_TYPE_DYNO:
        {
            SlaveVehMod = MODE_TYPE_DYNO;
            MstVehMod = MODE_TYPE_DYNO;
            break;
        }
        case MODE_TYPE_CRASH:
        {
            SlaveVehMod = MODE_TYPE_CRASH;
            MstVehMod = MODE_TYPE_CRASH;
            break;
        }
        case MODE_TYPE_FACTORY_PAUSED:
        {
            SlaveVehMod = MODE_TYPE_FACTORY_PAUSED;
            MstVehMod = MODE_TYPE_USER;
            break;
        }
        case MODE_TYPE_TRANSPORT_PAUSED:
        {
            SlaveVehMod = MODE_TYPE_TRANSPORT_PAUSED;
            MstVehMod = MODE_TYPE_USER;
            break;
        }
        default:
            return;
    } /* END OF SWITCH */
}

uint8 M7_0_VMM_Mode_Get(void)
{
    return s_Vehicle_FSM.curState;
}

static void fsmRegist(fsmType* pFsm, StateTable* pTable)
{
    pFsm->stateTable = pTable;
}

static void fsmStateTransfer(fsmType* pFsm, uint8 state)
{
    pFsm->curState = state;
}

static void fsmEventHandle(fsmType* pFsm, uint8 event)
{
    StateTable* pActTable = pFsm->stateTable;
    void (*eventActFun)(uint8) = NULL;
    uint8 NextState;
    uint8 CurState = pFsm->curState;
    uint8 maxNum = pFsm->size;
    uint8 change_flag = 0;
 
    for (uint8 i = 0; i < maxNum; i++)
    {
        if ((event == pActTable[i].event) && (CurState == pActTable[i].CurState))
        {
            change_flag = 1;
            eventActFun = pActTable[i].eventActFun;
            NextState = pActTable[i].NextState;
            break;
        }
    }

    if (change_flag)
    {
        if (eventActFun)
        {
            eventActFun(NextState);
        }

        fsmStateTransfer(pFsm, NextState);
        UpdateVehicleMode(NextState);
    }
    else
    {
        ;
    }
}

static M7_0_VMM_EVENT s_EventRecord = VMM_EVENT_NONE;
static void fsmEventSet(M7_0_VMM_EVENT Event)
{
    s_EventRecord = Event;
}

static M7_0_VMM_EVENT fsmEventGetAndClear(void)
{
    M7_0_VMM_EVENT temp = s_EventRecord;
    if (s_EventRecord != VMM_EVENT_NONE)
    {
        s_EventRecord = VMM_EVENT_NONE;
    }
    return temp;
}

static uint16 s_PowerOnOffCount = 0;
static uint32 s_TickFactoryPausedToTransport = 0;
static uint32 s_TickFactoryPausedToFactory = 0;
static uint8  s_FlagFactoryPausedTickStart = FALSE;
static uint32 s_TickHazardResetTiming = 0;
static uint8  s_FlagHazardResetTiming = FALSE;
static uint32 s_TickUserToDyno = 0;
static uint32 s_TickCrashToUser = 0;

static void SwitchControlTickAdd(void)
{
    if (s_TickFactoryPausedToTransport < VMM_U32_MAX)
    {
        s_TickFactoryPausedToTransport += TASK_CYCLE_PERIOD;
    }
    if (s_TickUserToDyno < VMM_U32_MAX)
    {
        s_TickUserToDyno += TASK_CYCLE_PERIOD;
    }
    if (s_TickCrashToUser < VMM_U32_MAX)
    {
        s_TickCrashToUser += TASK_CYCLE_PERIOD;
    }

    if (s_FlagFactoryPausedTickStart == TRUE)
    {
        if (s_TickFactoryPausedToFactory < VMM_U32_MAX)
        {
            s_TickFactoryPausedToFactory += TASK_CYCLE_PERIOD;
        }
    }
    else
    {
        s_TickFactoryPausedToFactory = 0;
    }

    if (s_FlagHazardResetTiming == TRUE)
    {
        if (s_TickHazardResetTiming < VMM_U32_MAX)
        {
            s_TickHazardResetTiming += TASK_CYCLE_PERIOD;
        }
    }
    else
    {
        s_TickHazardResetTiming = 0;
    }
}

static uint8 s_StatePowerOnOff = WAIT_FOR_POWER_OFF;
static void M7_0_VMM_UpdatePowerOnOffCount(void)
{
    uint16 PwrSt = M7_0_VMM_CheckPowerState();
    switch (s_StatePowerOnOff)
    {
    case WAIT_FOR_POWER_OFF:
        if (PwrSt == PM_OFF)
        {
            s_StatePowerOnOff = WAIT_FOR_POWER_ON;
        }
        break;

    case WAIT_FOR_POWER_ON:
        if (PwrSt == PM_ON)
        {
            if (s_PowerOnOffCount < VMM_U16_MAX)
            {
                s_PowerOnOffCount++;
            }
            s_StatePowerOnOff = WAIT_FOR_POWER_OFF;
        }
        break;

    default:
        break;
    }
}

static float M7_0_SpeedCalculate(uint8 Valid, uint16 SpeedSig)
{
    float SpeedValue = 0.0;
    if (Valid == SPEED_VALID)
    {
        SpeedValue = (float)SpeedSig * SPEED_FACTOR;
    }
    else
    {
        SpeedValue = 0.0;
    }

    return SpeedValue;
}

static void M7_0_UpdateInputValue(M7_0_VMM_CUR_VALUE *pt)
{
    uint8 SpeedValid = VehicleSpeedVld;
    uint16 SpeedSig = VehicleSpeed;
    pt->Speed = M7_0_SpeedCalculate(SpeedValid, SpeedSig);
    // IO
    pt->HazardSW = g_u16SENSOR_HazardSwIn;
    pt->BrakeSW = g_u16SENSOR_BrakePedalSwIn;
    pt->CrashIn = g_u16SENSOR_CrashIn;               /* 0-off 1-crash */
    
    pt->MasterLightSW = LightCombineSw;              /* 0-off 1-auto 2-position 3-lowbeam */
    pt->HighBeamSW = HighBeamFlashSw;                /* 0-inactive 1-active */
    // Model
    pt->PowerState = PowMode;
    pt->EngineState = EngineSts;
}

static uint8 M7_0_VMM_CheckSpeedStandstill(void)
{
    if (s_VMMValue.Speed < SPEED_THRESHOULD_STILL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

uint8 M7_0_VMM_CheckPowerState(void)
{
    return s_VMMValue.PowerState;
}

static M7_0_SUB_STATE s_StateFactoryPausedToTransport = SUB_STATE_NONE;
static void M7_0_FactoryPausedToTransportStateMachine(void)
{
    switch (s_StateFactoryPausedToTransport)
    {
        case SUB_STATE_NONE:
        {
            if (s_VMMValue.BrakeSW == FALSE)
            {
                s_StateFactoryPausedToTransport = SUB_STATE_STEP1;
            }
            /* tick count starts when brake is true */
            s_TickFactoryPausedToTransport = 0;
            break;
        }
        case SUB_STATE_STEP1:
        {
            if (s_VMMValue.BrakeSW == TRUE)
            {
                s_StateFactoryPausedToTransport = SUB_STATE_STEP2;
            }
            s_TickFactoryPausedToTransport = 0;
            break;
        }
        case SUB_STATE_STEP2:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.HighBeamSW == FALSE))
            {
                if (s_VMMValue.HazardSW == TRUE)
                {
                    s_StateFactoryPausedToTransport = SUB_STATE_STEP3;
                }
            }
            else
            {
                s_StateFactoryPausedToTransport = SUB_STATE_NONE;
            }
            break;
        }
        case SUB_STATE_STEP3:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.HighBeamSW == FALSE))
            {
                if (s_VMMValue.HazardSW == FALSE)
                {
                    s_StateFactoryPausedToTransport = SUB_STATE_STEP4;
                }
            }
            else
            {
                s_StateFactoryPausedToTransport = SUB_STATE_NONE;
            }
            break;
        }
        case SUB_STATE_STEP4:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.HighBeamSW == FALSE))
            {
                if (s_VMMValue.HazardSW == TRUE)
                {
                    s_StateFactoryPausedToTransport = SUB_STATE_STEP5;
                }
            }
            else
            {
                s_StateFactoryPausedToTransport = SUB_STATE_NONE;
            }
            break;
        }
        case SUB_STATE_STEP5:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.HighBeamSW == FALSE))
            {
                if (s_VMMValue.HazardSW == FALSE)
                {
                    s_StateFactoryPausedToTransport = SUB_STATE_STEP6;
                }
            }
            else
            {
                s_StateFactoryPausedToTransport = SUB_STATE_NONE;
            }
            break;
        }
        case SUB_STATE_STEP6:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.HazardSW == FALSE))
            {
                if (s_VMMValue.HighBeamSW == TRUE)
                {
                    s_StateFactoryPausedToTransport = SUB_STATE_STEP7;
                }
            }
            else
            {
                s_StateFactoryPausedToTransport = SUB_STATE_NONE;
            }
            break;
        }
        case SUB_STATE_STEP7:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.HazardSW == FALSE))
            {
                if (s_VMMValue.HighBeamSW == FALSE)
                {
                    s_StateFactoryPausedToTransport = SUB_STATE_STEP8;
                }
            }
            else
            {
                s_StateFactoryPausedToTransport = SUB_STATE_NONE;
            }
            break;
        }
        case SUB_STATE_STEP8:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.HazardSW == FALSE))
            {
                if (s_VMMValue.HighBeamSW == TRUE)
                {
                    s_StateFactoryPausedToTransport = SUB_STATE_STEP9;
                }
            }
            else
            {
                s_StateFactoryPausedToTransport = SUB_STATE_NONE;
            }
            break;
        }
        case SUB_STATE_STEP9:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.HazardSW == FALSE))
            {
                if (s_VMMValue.HighBeamSW == FALSE)
                {
                    fsmEventSet(VMM_EVENT_FACTORY_PAUSED_TO_TRANSPORT);
                    s_StateFactoryPausedToTransport = SUB_STATE_NONE;
                }
            }
            else
            {
                s_StateFactoryPausedToTransport = SUB_STATE_NONE;
            }
        }
        default:
            break;
    } /* END OF SWITCH */

    /* Reset state machine when time out */
    if (s_TickFactoryPausedToTransport > TIME_LMT_5SEC)
    {
        s_StateFactoryPausedToTransport = SUB_STATE_NONE;
        s_TickFactoryPausedToTransport = 0;
    }
}

static M7_0_SUB_STATE s_StateUserToDyno = SUB_STATE_NONE;
static void M7_0_UserToDynoStateMachine(void)
{
    switch (s_StateUserToDyno)
    {
        case SUB_STATE_NONE:
        {
            if (s_VMMValue.BrakeSW == FALSE)
            {
                s_StateUserToDyno = SUB_STATE_STEP1;
            }
            /* tick count starts when brake is true */
            s_TickUserToDyno = 0;
            break;
        }
        case SUB_STATE_STEP1:
        {
            if (s_VMMValue.BrakeSW == TRUE)
            {
                s_StateUserToDyno = SUB_STATE_STEP2;
            }
            s_TickUserToDyno = 0;
            break;
        }
        case SUB_STATE_STEP2:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.MasterLightSW == MASTER_LIGHT_OFF))
            {
                if (s_VMMValue.HazardSW == TRUE)
                {
                    s_StateUserToDyno = SUB_STATE_STEP3;
                }
            }
            else
            {
                s_StateUserToDyno = SUB_STATE_NONE;
            }
            break;
        }
        case SUB_STATE_STEP3:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.MasterLightSW == MASTER_LIGHT_OFF))
            {
                if (s_VMMValue.HazardSW == FALSE)
                {
                    s_StateUserToDyno = SUB_STATE_STEP4;
                }
            }
            else
            {
                s_StateUserToDyno = SUB_STATE_NONE;
            }
            break;
        }
        case SUB_STATE_STEP4:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.HazardSW == FALSE))
            {
                if (s_VMMValue.MasterLightSW != MASTER_LIGHT_OFF)
                {
                    s_StateUserToDyno = SUB_STATE_STEP5;
                }
            }
            else
            {
                s_StateUserToDyno = SUB_STATE_NONE;
            }
            break;
        }
        case SUB_STATE_STEP5:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.HazardSW == FALSE))
            {
                if (s_VMMValue.MasterLightSW == MASTER_LIGHT_OFF)
                {
                    s_StateUserToDyno = SUB_STATE_STEP6;
                }
            }
            else
            {
                s_StateUserToDyno = SUB_STATE_NONE;
            }
            break;
        }
        case SUB_STATE_STEP6:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.HazardSW == FALSE))
            {
                if (s_VMMValue.MasterLightSW != MASTER_LIGHT_OFF)
                {
                    s_StateUserToDyno = SUB_STATE_STEP7;
                }
            }
            else
            {
                s_StateUserToDyno = SUB_STATE_NONE;
            }
            break;
        }
        case SUB_STATE_STEP7:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.HazardSW == FALSE))
            {
                if (s_VMMValue.MasterLightSW == MASTER_LIGHT_OFF)
                {
                    s_StateUserToDyno = SUB_STATE_STEP8;
                }
            }
            else
            {
                s_StateUserToDyno = SUB_STATE_NONE;
            }   
            break;
        }
        case SUB_STATE_STEP8:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.MasterLightSW == MASTER_LIGHT_OFF))
            {
                if (s_VMMValue.HazardSW == TRUE)
                {
                    s_StateUserToDyno = SUB_STATE_STEP9;
                }
            }
            else
            {
                s_StateUserToDyno = SUB_STATE_NONE;
            }
            break;
        }
        case SUB_STATE_STEP9:
        {
            if ((s_VMMValue.BrakeSW == TRUE)&&(s_VMMValue.MasterLightSW == MASTER_LIGHT_OFF))
            {
                if (s_VMMValue.HazardSW == FALSE)
                {
                    fsmEventSet(VMM_EVENT_USER_TO_DYNO);
                    s_StateUserToDyno = SUB_STATE_NONE;
                }
            }
            else
            {
                s_StateUserToDyno = SUB_STATE_NONE;
            }
            break;
        }
        default:
            break;
    } /* END OF SWITCH */

    /* Reset state machine when time out */
    if (s_TickUserToDyno > TIME_LMT_5SEC)
    {
        s_StateUserToDyno = SUB_STATE_NONE;
        s_TickUserToDyno = 0;
    }
}

static M7_0_SUB_STATE s_StateCrashToUser = SUB_STATE_NONE;
static void M7_0_CrashToUserStateMachine(void)
{
    switch (s_StateCrashToUser)
    {
        case SUB_STATE_NONE:
        {
            if (s_VMMValue.PowerState == PM_OFF)
            {
                s_StateCrashToUser = SUB_STATE_STEP1;
            }
            /* tick count starts when brake is true */
            s_TickCrashToUser = 0;
            break;
        }
        case SUB_STATE_STEP1:
        {
            if (s_VMMValue.PowerState == PM_ON)
            {
                s_StateCrashToUser = SUB_STATE_STEP2;
            }
            break;
        }
        case SUB_STATE_STEP2:
        {
            if (s_VMMValue.PowerState == PM_OFF)
            {
                s_StateCrashToUser = SUB_STATE_STEP3;
            }
            break;
        }
        case SUB_STATE_STEP3:
        {
            if (s_VMMValue.PowerState == PM_ON)
            {
                fsmEventSet(VMM_EVENT_CRASH_TO_USER);
                s_StateCrashToUser = SUB_STATE_NONE;
            }
            break;
        }
        default:
            break;
    } /* END OF SWITCH */

    /* Reset state machine when time out */
    if (s_TickCrashToUser > TIME_LMT_10SEC)
    {
        s_StateCrashToUser = SUB_STATE_NONE;
        s_TickCrashToUser = 0;
    }
}

static M7_0_FIND_KEY_STATE s_FindKeyState = FIND_KEY_START;
static M7_0_VMM_EVENT s_FindKeyEventRecord = VMM_EVENT_NONE;
static void FindKeyStart(M7_0_VMM_EVENT Event)
{
    s_FindKeyEventRecord = Event;
} 

void M7_0_FindKeyTriggerEvent(void)
{
    switch (s_FindKeyState)
    {
        case FIND_KEY_START:
        {
            if (s_FindKeyEventRecord != VMM_EVENT_NONE)
            {
                s_FindKeyState = FIND_KEY_WAIT_FOR_SUCCESS;
            }
            break;
        }
        case FIND_KEY_WAIT_FOR_SUCCESS:
        {
            /* PEPS find key in 1s */
            break;
        }
        case FIND_KEY_SUCCESS:
        {
            fsmEventSet(s_FindKeyEventRecord);
            s_VMM_DiagInfo.State = ROUTINE_COMPLETED;
            s_FindKeyEventRecord = VMM_EVENT_NONE;
            break;
        }
        case FIND_KEY_TIME_OUT:
        {
            /* find key failed */
            s_VMM_DiagInfo.State = ROUTINE_FAILURE;
            s_VMM_DiagInfo.FailReason.bit.KeyInvalid = 1;

            s_FindKeyEventRecord = VMM_EVENT_NONE;
            break;
        }
    }
}

static M7_0_SUB_STATE s_StateDynoToUser = SUB_STATE_NONE;
static M7_0_SUB_STATE s_StateTransportToTransportPaused = SUB_STATE_NONE;
static M7_0_SUB_STATE s_StateTransportPausedToTransport = SUB_STATE_NONE;
void M7_0_VMM_FiniteStateMachine(void)
{
    switch(s_Vehicle_FSM.curState)
    {
        case MODE_TYPE_FACTORY:
        {
            if ((M7_0_VMM_CheckSpeedStandstill() == TRUE)&&(M7_0_VMM_CheckPowerState() != PM_OFF))
            {
                fsmEventSet(VMM_EVENT_FACTORY_TO_FACTORY_PAUSED);
            }
            break;
        }
        case MODE_TYPE_FACTORY_PAUSED:
        {
            if ((M7_0_VMM_CheckSpeedStandstill() == TRUE)&&((M7_0_VMM_CheckPowerState() == PM_ON)&&(s_VMMValue.EngineState == ENG_NOT_RUNNING)))
            {
                M7_0_FactoryPausedToTransportStateMachine();
            }
            else
            {
                /* Reset sub state */
                s_StateFactoryPausedToTransport = SUB_STATE_NONE;
                s_TickFactoryPausedToTransport = 0;

                if (s_TickFactoryPausedToFactory > TIME_LMT_3MIN)
                {
                    fsmEventSet(VMM_EVENT_FACTORY_PAUSED_TO_FACTORY);
                }
            }
            break;
        }
        case MODE_TYPE_TRANSPORT:
        {
            if (M7_0_VMM_CheckSpeedStandstill() == TRUE)
            {
                /* transport to transport paused start */
                /* Power not on -> on && Engine not running -> running */
                if ((s_StateTransportToTransportPaused == SUB_STATE_NONE)&&(M7_0_VMM_CheckPowerState() != PM_ON)&&(s_VMMValue.EngineState == ENG_NOT_RUNNING))
                {
                    s_StateTransportToTransportPaused = SUB_STATE_STEP1;
                }
                else if ((s_StateTransportToTransportPaused == SUB_STATE_STEP1)&&(M7_0_VMM_CheckPowerState() == PM_ON)&&(s_VMMValue.EngineState == ENG_RUNNING))
                {
                    fsmEventSet(VMM_EVENT_TRANSPORT_TO_TRANPORT_PAUSED);
                    s_StateTransportToTransportPaused = SUB_STATE_NONE;
                }
                /* transport to transport paused end */
            
                if (s_PowerOnOffCount >= POWER_ON_OFF_CNT_50)
                {
                    fsmEventSet(VMM_EVENT_TRANSPORT_TO_USER);
                }
            }
            else
            {
                s_StateTransportToTransportPaused = SUB_STATE_NONE;
            }
            break;
        }
        case MODE_TYPE_TRANSPORT_PAUSED:
        {
            if (M7_0_VMM_CheckSpeedStandstill() == TRUE)
            {
                /* transport to transport paused start */
                /* Power on -> not on && Engine running -> not running */
                if ((s_StateTransportPausedToTransport == SUB_STATE_NONE)&&(M7_0_VMM_CheckPowerState() == PM_ON)&&(s_VMMValue.EngineState == ENG_RUNNING))
                {
                    s_StateTransportPausedToTransport = SUB_STATE_STEP1;
                }
                else if ((s_StateTransportPausedToTransport == SUB_STATE_STEP1)&&(M7_0_VMM_CheckPowerState() != PM_ON)&&(s_VMMValue.EngineState == ENG_NOT_RUNNING))
                {
                    fsmEventSet(VMM_EVENT_FACTORY_PAUSED_TO_TRANSPORT);
                    s_StateTransportPausedToTransport = SUB_STATE_NONE;
                }
                /* transport to transport paused end */  
            }
            else
            {
                s_StateTransportPausedToTransport = SUB_STATE_NONE;
            }
            break;
        }
        case MODE_TYPE_DYNO:
        {
            if ((M7_0_VMM_CheckSpeedStandstill() == TRUE)&&((M7_0_VMM_CheckPowerState() != PM_ON)))
            {
                /* Dyno to user start */
                if ((s_VMMValue.EngineState == ENG_RUNNING)&&(s_StateDynoToUser == SUB_STATE_NONE))
                {
                    s_StateDynoToUser = SUB_STATE_STEP1;
                }
                else if ((s_VMMValue.EngineState == ENG_NOT_RUNNING)&&(s_StateDynoToUser == SUB_STATE_STEP1))
                {
                    s_StateDynoToUser = SUB_STATE_NONE;
                    fsmEventSet(VMM_EVENT_DYNO_TO_USER);
                }
                /* Dyno to user end */
            }
            else 
            {
                /* Reset engine states */
                s_StateDynoToUser = SUB_STATE_NONE;
            }
            break;
        }
        case MODE_TYPE_USER:
        {
            if ((M7_0_VMM_CheckSpeedStandstill() == TRUE)&&((M7_0_VMM_CheckPowerState() == PM_ON)&&(s_VMMValue.EngineState == ENG_RUNNING)))
            {
                M7_0_UserToDynoStateMachine();             
            }
            else
            {
                /* Reset sub state */
                s_StateUserToDyno = SUB_STATE_NONE;
                s_TickUserToDyno = 0;
            }
            break;
        }
        case MODE_TYPE_CRASH:
        {
            if (M7_0_VMM_CheckSpeedStandstill() == TRUE)
            {
                /* power off-on-off-on in 10s */
                M7_0_CrashToUserStateMachine();
            }
            else
            {
                /* Reset sub state */
                s_StateCrashToUser = SUB_STATE_NONE;
                s_TickCrashToUser = 0;

            }
            break;
        }
        default:
        {
            break;
        }
    }

    /* Independent Event */
    if (s_Vehicle_FSM.curState != MODE_TYPE_CRASH)
    {
        if (s_VMMValue.CrashIn == TRUE)
        {
            fsmEventSet(VMM_EVENT_CRASH);
        }
    }
    else if ((s_Vehicle_FSM.curState == MODE_TYPE_FACTORY)||(s_Vehicle_FSM.curState == MODE_TYPE_FACTORY_PAUSED))
    {
        if ((M7_0_VMM_CheckSpeedStandstill() == TRUE)&&(s_PowerOnOffCount >= POWER_ON_OFF_CNT_100))
        {
            fsmEventSet(VMM_EVENT_FACTORY_TO_USER);
        }
    }

    /* Tick adds in this condition */
    if ((s_Vehicle_FSM.curState == MODE_TYPE_FACTORY_PAUSED)&&(M7_0_VMM_CheckPowerState() == PM_OFF))
    {
        s_FlagFactoryPausedTickStart = TRUE;
        /* Hazard SW valid lasts 3s, reset 3Min timing */
        if (s_VMMValue.HazardSW == TRUE)
        {
            s_FlagHazardResetTiming = TRUE;
        }
        else
        {
            s_FlagHazardResetTiming = FALSE;
        }

        if (s_TickHazardResetTiming > TIME_LMT_3SEC)
        {
            s_TickHazardResetTiming = TIME_LMT_3SEC;
            s_TickFactoryPausedToFactory = 0;
        }
    }
    else
    {
        s_FlagFactoryPausedTickStart = FALSE;
        s_FlagHazardResetTiming = FALSE;
    }

    /* Reset sub state */
    if (s_Vehicle_FSM.curState != MODE_TYPE_FACTORY_PAUSED)
    {
        s_StateFactoryPausedToTransport = SUB_STATE_NONE;
        s_TickFactoryPausedToTransport = 0;
    }
    if (s_Vehicle_FSM.curState != MODE_TYPE_USER)
    {
        s_StateUserToDyno = SUB_STATE_NONE;
        s_TickUserToDyno = 0;
    }
    if (s_Vehicle_FSM.curState != MODE_TYPE_CRASH)
    {
        s_StateCrashToUser = SUB_STATE_NONE;
        s_TickCrashToUser = 0;
    }
    if (s_Vehicle_FSM.curState != MODE_TYPE_DYNO)
    {
        s_StateDynoToUser = SUB_STATE_NONE;
    }
    if (s_Vehicle_FSM.curState != MODE_TYPE_TRANSPORT)
    {
        s_StateTransportToTransportPaused = SUB_STATE_NONE;
    }
    if (s_Vehicle_FSM.curState != MODE_TYPE_TRANSPORT_PAUSED)
    {
        s_StateTransportPausedToTransport = SUB_STATE_NONE;
    }
}

/* Write vehicle mode to EEPROM */
uint8 UpdateVehicleMode(uint8 vehicle_mode)
{
    return 0;
}

/* Read mode from EEP, init FSM */
void M7_0_Vehicle_Mode_Init(void)
{
    // Start of user code for the runnable of M7_0_Vehicle_Mode_Init
    uint8 init_vehicle_mode = MODE_TYPE_INIT;
    memset(&s_VMMValue, 0, sizeof(M7_0_VMM_CUR_VALUE));
    M7_0_UpdateInputValue(&s_VMMValue);

    /* initialize vehicle mode form EEPROM */

    // default mode : Factory mode
    if(init_vehicle_mode == MODE_TYPE_INIT)
    {
        init_vehicle_mode = MODE_TYPE_FACTORY;
        /* Write to EEPROM */
        UpdateVehicleMode(init_vehicle_mode);
    }
    Vehicle_Mode_SW_Handle(init_vehicle_mode);

    fsmRegist(&s_Vehicle_FSM,(StateTable *)Vehicle_FSM_Table);
    s_Vehicle_FSM.curState = init_vehicle_mode;
    s_Vehicle_FSM.size = sizeof(Vehicle_FSM_Table)/sizeof(StateTable);
    // End of user code
}


static void M7_0_VMM_EventHandle (void)
{
    // Start of user code for the runnable of Vehicle_Mode_Manager
    M7_0_VMM_EVENT event = VMM_EVENT_NONE;
    event = fsmEventGetAndClear();

    /* if event occurs, clear power on-off conut, except switching between factory and factory paused */  
    if((event != VMM_EVENT_NONE)&&
       (event != VMM_EVENT_FACTORY_PAUSED_TO_FACTORY)&&
       (event != VMM_EVENT_FACTORY_TO_FACTORY_PAUSED))
    {
        s_PowerOnOffCount = 0;
    }

    fsmEventHandle(&s_Vehicle_FSM, event);

    // End of user code
}

/* Cycled run task */
void M7_0_VehicleModeManagerRunnable(void)
{
    // Start of user code for the runnable of M7_0_VehicleModeManagerRunnable
    /* get io input value & speed & power & engine state */
    M7_0_UpdateInputValue(&s_VMMValue);
    
    M7_0_FindKeyTriggerEvent();
    SwitchControlTickAdd();
    M7_0_VMM_UpdatePowerOnOffCount();

    M7_0_VMM_DiagnosticHandle();
    M7_0_VMM_FiniteStateMachine();
    M7_0_VMM_EventHandle();
    // End of user code
}

void M7_0_VMM_DiagnosticHandle(void)
{
    if (s_VMM_DiagInfo.State == ROUTINE_START)
    {
        s_VMM_DiagInfo.State = ROUTINE_IN_PROGRESS;
    }
    else if (s_VMM_DiagInfo.State == ROUTINE_IN_PROGRESS)
    {
        ;
    }
    else
    {
        return;
    }

    if (s_VMM_DiagInfo.RequestMode == TEL_DIAG_NONE)
    {
        return;
    }

    switch (s_VMM_DiagInfo.RequestMode)
    {
        case TEL_DIAG_FACTORY_PAUSED_MODE:
        {
            if ((M7_0_VMM_CheckSpeedStandstill() == TRUE)&&(M7_0_VMM_CheckPowerState() == PM_ON)&&(s_VMMValue.EngineState == ENG_NOT_RUNNING))
            {
                if (s_Vehicle_FSM.curState == MODE_TYPE_TRANSPORT)
                {
                    fsmEventSet(VMM_EVENT_TRANSPORT_TO_FACTORY_PAUSED);
                    s_VMM_DiagInfo.State = ROUTINE_COMPLETED;
                }
                else if (s_Vehicle_FSM.curState == MODE_TYPE_USER)
                {
                    /* PEPS find key in 1s */
                    FindKeyStart(VMM_EVENT_USER_TO_FACTORY_PAUSED);
                }
                else if (s_Vehicle_FSM.curState == MODE_TYPE_CRASH)
                {
                    /* PEPS find key in 1s */
                    FindKeyStart(VMM_EVENT_CRASH_TO_FACTORY_PAUSED);
                }
                else
                {
                    s_VMM_DiagInfo.State = ROUTINE_FAILURE;
                    s_VMM_DiagInfo.FailReason.bit.CurModeWrong = 1;
                }
            }
            else
            {
                if (M7_0_VMM_CheckSpeedStandstill() != TRUE)
                {
                    s_VMM_DiagInfo.FailReason.bit.SpeedMisMatch = 1;
                }
                if ((M7_0_VMM_CheckPowerState() != PM_ON)||(s_VMMValue.EngineState != ENG_NOT_RUNNING))
                {
                    s_VMM_DiagInfo.FailReason.bit.PwrModMisMatch = 1;
                }
                if ((s_Vehicle_FSM.curState != MODE_TYPE_TRANSPORT)&&
                    (s_Vehicle_FSM.curState != MODE_TYPE_USER)&&
                    (s_Vehicle_FSM.curState != MODE_TYPE_CRASH))
                {
                    s_VMM_DiagInfo.FailReason.bit.CurModeWrong = 1;
                }
                s_VMM_DiagInfo.State = ROUTINE_FAILURE;
            }
            break;
        }

        case TEL_DIAG_TRANSPORT_MODE:
        {
            if ((M7_0_VMM_CheckSpeedStandstill() == TRUE)&&(M7_0_VMM_CheckPowerState() == PM_ON)&&(s_VMMValue.EngineState == ENG_NOT_RUNNING))
            {
                if (s_Vehicle_FSM.curState == MODE_TYPE_FACTORY_PAUSED)
                {
                    fsmEventSet(VMM_EVENT_FACTORY_PAUSED_TO_TRANSPORT);
                    s_VMM_DiagInfo.State = ROUTINE_COMPLETED;
                }
                else if (s_Vehicle_FSM.curState == MODE_TYPE_USER)
                {
                    /* PEPS find key in 1s */
                    FindKeyStart(VMM_EVENT_USER_TO_TRANSPORT);
                }
                else if (s_Vehicle_FSM.curState == MODE_TYPE_CRASH)
                {
                    /* PEPS find key in 1s */
                    FindKeyStart(VMM_EVENT_CRASH_TO_TRANSPORT);
                }
                else
                {
                    s_VMM_DiagInfo.State = ROUTINE_FAILURE;
                    s_VMM_DiagInfo.FailReason.bit.CurModeWrong = 1;
                }
            }
            else
            {
                if (M7_0_VMM_CheckSpeedStandstill() != TRUE)
                {
                    s_VMM_DiagInfo.FailReason.bit.SpeedMisMatch = 1;
                }
                if ((M7_0_VMM_CheckPowerState() != PM_ON)||(s_VMMValue.EngineState != ENG_NOT_RUNNING))
                {
                    s_VMM_DiagInfo.FailReason.bit.PwrModMisMatch = 1;
                }
                if ((s_Vehicle_FSM.curState != MODE_TYPE_FACTORY_PAUSED)&&
                    (s_Vehicle_FSM.curState != MODE_TYPE_USER)&&
                    (s_Vehicle_FSM.curState != MODE_TYPE_CRASH))
                {
                    s_VMM_DiagInfo.FailReason.bit.CurModeWrong = 1;
                }
                s_VMM_DiagInfo.State = ROUTINE_FAILURE;
            }
            break;
        }

        case TEL_DIAG_USER_MODE:
        {
            if ((s_Vehicle_FSM.curState != MODE_TYPE_FACTORY_PAUSED)&&
                (s_Vehicle_FSM.curState != MODE_TYPE_TRANSPORT)&&
                (s_Vehicle_FSM.curState != MODE_TYPE_DYNO)&&
                (s_Vehicle_FSM.curState != MODE_TYPE_CRASH))
            {
                s_VMM_DiagInfo.FailReason.bit.CurModeWrong = 1;
                s_VMM_DiagInfo.State = ROUTINE_FAILURE;
            }
            if (M7_0_VMM_CheckSpeedStandstill() != TRUE)
            {
                s_VMM_DiagInfo.FailReason.bit.SpeedMisMatch = 1;
                s_VMM_DiagInfo.State = ROUTINE_FAILURE;
            }
            
            if (s_VMM_DiagInfo.State != ROUTINE_FAILURE)
            {
                if (s_Vehicle_FSM.curState == MODE_TYPE_FACTORY_PAUSED)
                {
                    if ((M7_0_VMM_CheckPowerState() == PM_ON)&&(s_VMMValue.EngineState == ENG_NOT_RUNNING))
                    {
                        fsmEventSet(VMM_EVENT_FACTORY_PAUSED_TO_USER);
                        s_VMM_DiagInfo.State = ROUTINE_COMPLETED;
                    }
                    else
                    {
                        s_VMM_DiagInfo.FailReason.bit.PwrModMisMatch = 1;
                        s_VMM_DiagInfo.State = ROUTINE_FAILURE;
                    }
                }
                else if (s_Vehicle_FSM.curState == MODE_TYPE_TRANSPORT)
                {
                    if ((M7_0_VMM_CheckPowerState() == PM_ON)&&(s_VMMValue.EngineState == ENG_NOT_RUNNING))
                    {
                        fsmEventSet(VMM_EVENT_TRANSPORT_TO_USER);
                        s_VMM_DiagInfo.State = ROUTINE_COMPLETED;
                    }
                    else
                    {
                        s_VMM_DiagInfo.FailReason.bit.PwrModMisMatch = 1;
                        s_VMM_DiagInfo.State = ROUTINE_FAILURE;
                    }
                }
                else if (s_Vehicle_FSM.curState == MODE_TYPE_DYNO)
                {
                    if ((M7_0_VMM_CheckPowerState() == PM_ON)&&(s_VMMValue.EngineState == ENG_RUNNING))
                    {
                        fsmEventSet(VMM_EVENT_DYNO_TO_USER);
                        s_VMM_DiagInfo.State = ROUTINE_COMPLETED;
                    }
                    else
                    {
                        s_VMM_DiagInfo.FailReason.bit.PwrModMisMatch = 1;
                        s_VMM_DiagInfo.State = ROUTINE_FAILURE;
                    }
                }
                else if (s_Vehicle_FSM.curState == MODE_TYPE_CRASH)
                {
                    if ((M7_0_VMM_CheckPowerState() == PM_ON)&&(s_VMMValue.EngineState == ENG_NOT_RUNNING))
                    {
                        fsmEventSet(VMM_EVENT_CRASH_TO_USER);
                        s_VMM_DiagInfo.State = ROUTINE_COMPLETED;
                    }
                    else
                    {
                        s_VMM_DiagInfo.FailReason.bit.PwrModMisMatch = 1;
                        s_VMM_DiagInfo.State = ROUTINE_FAILURE;
                    }
                }
                else
                {
                    s_VMM_DiagInfo.FailReason.bit.CurModeWrong = 1;
                    s_VMM_DiagInfo.State = ROUTINE_FAILURE;
                }
            }
            break;
        }

        case TEL_DIAG_DYNO_MODE:
        {
            if ((s_Vehicle_FSM.curState == MODE_TYPE_USER)&&(M7_0_VMM_CheckSpeedStandstill() == TRUE)&&((M7_0_VMM_CheckPowerState() == PM_ON)&&(s_VMMValue.EngineState == ENG_RUNNING)))
            {
                fsmEventSet(VMM_EVENT_USER_TO_DYNO);
                s_VMM_DiagInfo.State = ROUTINE_COMPLETED;
            }
            else
            {
                if (s_Vehicle_FSM.curState != MODE_TYPE_USER)
                {
                    s_VMM_DiagInfo.FailReason.bit.CurModeWrong = 1;
                }
                if (M7_0_VMM_CheckSpeedStandstill() != TRUE)
                {
                    s_VMM_DiagInfo.FailReason.bit.SpeedMisMatch = 1;
                }
                if ((M7_0_VMM_CheckPowerState() == PM_ON)&&(s_VMMValue.EngineState == ENG_RUNNING))
                {
                    s_VMM_DiagInfo.FailReason.bit.PwrModMisMatch = 1;
                }
                s_VMM_DiagInfo.State = ROUTINE_FAILURE;
            }
            break;
        }

        default:
        {
            break;
        }
    } /* END OF SWITCH */

    /* clear receive diagnostic tel buff */
    s_VMM_DiagInfo.RequestMode = TEL_DIAG_NONE;
}

void M7_0_VMM_DiagRequestStart(uint8 mode)
{
    uint8 ModeIndex;
    uint8 MatchCnt = 0;

    for (ModeIndex = TEL_DIAG_FACTORY_PAUSED_MODE; ModeIndex < TEL_DIAG_MAX; ModeIndex++)
    {
        if (ModeIndex == mode)
        {
            if ((s_VMM_DiagInfo.State != ROUTINE_START)&&(s_VMM_DiagInfo.State != ROUTINE_IN_PROGRESS))
            {
                s_VMM_DiagInfo.State = ROUTINE_START;
                s_VMM_DiagInfo.RequestMode = mode;
                s_VMM_DiagInfo.FailReason.value = 0;
            }
            MatchCnt++;
            break;
        }
    }
    /* no match for diag mode */
    if (MatchCnt == 0)
    {
        if ((s_VMM_DiagInfo.State != ROUTINE_START)&&(s_VMM_DiagInfo.State != ROUTINE_IN_PROGRESS))
        {
            s_VMM_DiagInfo.State = ROUTINE_FAILURE;
            s_VMM_DiagInfo.RequestMode = TEL_DIAG_NONE;
            s_VMM_DiagInfo.FailReason.bit.RequestModeErr = 1;
        }
    }
}

void M7_0_VMM_DiagRequestStop(void)
{
    if (s_VMM_DiagInfo.State != ROUTINE_STOPPED)
    {
        s_VMM_DiagInfo.State = ROUTINE_STOPPED;
        s_VMM_DiagInfo.RequestMode = TEL_DIAG_NONE;
        s_VMM_DiagInfo.FailReason.value = 0;
    }
}

void M7_0_VMM_DiagInquiry(VMM_RoutineInquiryData *DiagReturnValue)
{
    if (DiagReturnValue == NULL)
    {
        return;
    }

    DiagReturnValue->DiagStatus = s_VMM_DiagInfo.State;
    if(s_VMM_DiagInfo.State != ROUTINE_FAILURE)
    {
        DiagReturnValue->FailReason = s_VMM_DiagInfo.FailReason.value;
    }
    else
    {
        DiagReturnValue->FailReason = 0;
    }
}
