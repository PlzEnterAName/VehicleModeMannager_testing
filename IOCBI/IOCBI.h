 /************************Copyright by ofilm 2020-2030*****************************
  * @file    IOCBI.h 
  * @author  Application Team
  * @version V1.0.0
  * @date    8-6-2023
  * @brief   IOCBI Module
  *******************************************************************************/
#ifndef __M7_0_IOCBI_H_
#define __M7_0_IOCBI_H_

#define IOCBI_CM7_0_VERSION     IOCBI_CM7_0_000002

#define NULL ((void *)0)

#ifndef ON
#define ON (1U)
#endif

#ifndef OFF
#define OFF (0U)
#endif

// #define IOCBI_CALL_PERIOD                     10u /* ms */
#define IOCBI_NO_SAFE_TIME                    0x00u
#define IOCBI_SAFE_TIME_6SEC                  6000u /* ms */
#define IOCBI_SAFE_TIME_60SEC                 60000u /* ms */

#define IOCBI_SUPPORT_FREEZE                  OFF
#define IOCBI_INVAILD_DID_INDEX               0xFFu
#define IOCBI_RETURNCONTROLTOECU              0x00u
#define IOCBI_FREEZECONTROLSTATUS             0x02u
#define IOCBI_SHORTTERMADJUSTMENT             0x03u

#define IOCBI_INVAILD_CHANNELID_INDEX         0xFFFFu
#define IOCBI_RELATED_CHANNELID_INDEX         0xFFFEu

#define IOCBI_CHANNEL_DIO   0
#define IOCBI_CHANNEL_PWM   1

/*  Type Define Section                                                       */
typedef unsigned char U8;   
typedef unsigned short U16; 
typedef unsigned long U32; 
typedef unsigned long long U64;

#define IOCBI_U32_MAX 0xFFFFFFFF

/* Special return codes */
#define S_DSM_RC_OK      (0x01u)/* process ok */
#define S_DSM_RC_PENDING (0x02u)/* pending */

/* Negative return codes */
#define S_DSM_RC_GR      (0x10u)/* generalReject */
#define S_DSM_RC_SNS     (0x11u)/* serviceNotSupported */
#define S_DSM_RC_SFNS    (0x12u)/* subFunctionNotSupported */
#define S_DSM_RC_IMLOIF  (0x13u)/* incorrectMessageLengthOrInvalidFormat */
#define S_DSM_RC_RTL     (0x14u)/* responseTooLong */
#define S_DSM_RC_BRR     (0x21u)/* busyRepeatRequest */
#define S_DSM_RC_CNC     (0x22u)/* conditionsNotCorrect */
#define S_DSM_RC_RSE     (0x24u)/* requestSequenceError */
#define S_DSM_RC_FPERA   (0x26u)/* failurePreventsExecutionOfRequestedAction*/
#define S_DSM_RC_ROOR    (0x31u)/* requestOutOfRange */
#define S_DSM_RC_SAD     (0x33u)/* securityAccessDenied */
#define S_DSM_RC_IK      (0x35u)/* invalidKey */
#define S_DSM_RC_ENOA    (0x36u)/* exceedNumberOfAttempts */
#define S_DSM_RC_RTDNE   (0x37u)/* requiredTimeDelayNotExpired */
#define S_DSM_RC_UDNA    (0x70u)/* uploadDownloadNotAccepted */
#define S_DSM_RC_TDS     (0x71u)/* transferDataSuspended */
#define S_DSM_RC_GPF     (0x72u)/* generalProgrammingFailure */
#define S_DSM_RC_WBSC    (0x73u)/* wrongBlockSequenceCounter */
#define S_DSM_RC_RCRRP   (0x78u)/* requestCorrectlyReceived-ResponsePending */
#define S_DSM_RC_SFNSIAS (0x7Eu)/* subFunctionNotSupportedInActiveSession */
#define S_DSM_RC_SNSIAS  (0x7Fu)/* serviceNotSupportedInActiveSession */
#define S_DSM_RC_RPMTH   (0x81u)/* rpmTooHigh */
#define S_DSM_RC_RPMTL   (0x82u)/* rpmTooLow */
#define S_DSM_RC_EIR     (0x83u)/* engineIsRunning */
#define S_DSM_RC_EINR    (0x84u)/* engineIsNotRunning */
#define S_DSM_RC_ERTTL   (0x85u)/* engineRunTimeTooLow */
#define S_DSM_RC_TEMPTH  (0x86u)/* temperatureTooHigh */
#define S_DSM_RC_TEMPTL  (0x87u)/* temperatureTooLow */
#define S_DSM_RC_VSTH    (0x88u)/* vehicleSpeedTooHigh */
#define S_DSM_RC_VSTL    (0x89u)/* vehicleSpeedTooLow */
#define S_DSM_RC_TPTH    (0x8Au)/* throttle/PedalTooHigh */
#define S_DSM_RC_TPTL    (0x8Bu)/* throttle/PedalTooLow */
#define S_DSM_RC_TRNIN   (0x8Cu)/* transmissionRangeNotInNeutral */
#define S_DSM_RC_BSNC    (0x8Fu)/* brakeSwitch(es)NotClosed */
#define S_DSM_RC_SLNIP   (0x90u)/* shifterLeverNotInPark */
#define S_DSM_RC_TCCL    (0x91u)/* torqueConverterClutchLocked */
#define S_DSM_RC_VTH     (0x92u)/* voltageTooHigh */
#define S_DSM_RC_VTL     (0x93u)/* voltageTooLow */

/******************************************************************************/
/*  Type Define Section                                                       */
/******************************************************************************/
typedef enum  //<----------Need to config
{
    /* 0x600D */
    ID600D_Left_turn_lamp = 0,
    /* 0x600E */
    ID600E_Right_turn_lamp,
    /* 0x600F */
    ID600F_Position_lamp_Backlight,
    ID600F_Position_lamp_License_lamp,
    /* 0x6010 */
    ID6010_FL_Low_beam,
    ID6010_FR_Low_beam,
    /* 0x6011 */
    ID6011_FL_High_beam,
    ID6011_FR_High_beam,
    /* 0x6012 */
    ID6012_FL_Fog_lamp,
    ID6012_FR_Fog_lamp,
    /* 0x6013 */
    ID6013_Rear_Fog_lamp,
    /* 0x6015 */
    ID6015_L_Daytime_running_lamp,
    ID6015_R_Daytime_running_lamp,
    /* 0x6016 */
    ID6016_Interrior_Lamps,
    /* 0x6017 */
    ID6017_Welcome_lamp_RL,
    ID6017_Welcome_lamp_FL,
    ID6017_Welcome_lamp_RR,
    ID6017_Welcome_lamp_FR,
    /* 0x6018 Canceled */
    /* 0x6019 */
    ID6019_Hazard_indicator,
    ID6019_Left_turn_lamp,
    ID6019_Right_turn_lamp,
    /* 0x601A */
    ID601A_Brake_lamp_RL_RR,
    ID601A_HIGH_Brake_lamp,
    /* 0x601B */
    ID601B_Front_Wiper_Low,
    ID601B_Front_Wiper_High,
    /* 0x601C */
    ID601C_FrontWasher,
    /* 0x601D */
    ID601D_Front_Wiper_Low,
    /* 0x601E */
    ID601E_FL_Window_Up_Ctrl,
    ID601E_FL_Window_Down_Ctrl,
    /* 0x601F */
    ID601F_FR_Window_Up_Ctrl,
    ID601F_FR_Window_Down_Ctrl,
    /* 0x6021 */
    ID6021_RL_Window_Up_Ctrl,
    ID6021_RL_Window_Down_Ctrl,
    /* 0x6022 */
    ID6022_RR_Window_Up_Ctrl,
    ID6022_RR_Window_Down_Ctrl,
    /* 0x6023 */
    ID6023_Drive_Door_Motor_Fuel_lid_unLockCtrl_EN_C,
    ID6023_FR_RL_RR_Door_Motor_UnlockCtrl_EN_C,
    ID6023_ALL_Door_Motor_Lock_Ctrl_EN_C,
    ID6023_Electric_child_unLock_EN_C,
    /* 0x6024 */
    ID6024_Trunk_release_EN_C,
    /* 0x6025 */ 
    ID6025_Horn,
    ID6025_Siren,
    /* 0x6027 */
    ID6027_Mirror_fold_EN_C,
    ID6027_Mirror_unfold_EN_C,
    /* 0x6028 */
    ID6028_Rear_Windscreen_Mirror_Heating,
    /* 0x602A */
    ID602A_Passenger_inhibit_Electric_child_protection_indicator,
    /* 0x602B */
    ID602B_Trunk_Lamp,
    /* 0x6031 */
    ID6031_Front_Windscreen_Heating1,
    ID6031_Front_Windscreen_Heating2,
    /* 0x6032 */
    ID6032_Steering_Wheel_Heating,
    ID6032_Steering_wheel_heating_indicator,
    /* 0x6033 */
    ID6033_Nozzle_heating,
    /* 0x6034 */
    ID6034_Headlamp_height_adjustment,
    /* 0x6035 */
    ID6035_Backlight_adjustment,
    /* 0x6038 */
    ID6038_Electric_child_unLock_EN_C,
    /* 0x603B */
    ID603B_FL_Fog_lamp,
    /* 0x603C */
    ID603C_FR_Fog_lamp,
    /* 0x603D */
    ID603D_Lock_status_indication,
    /* 0x6045 */
    ID6045_L_Cornering_Lamp,
    /* 0x6046 */
    ID6046_R_Cornering_Lamp,
    /* 0x6047 */
    ID6047_Rearview_mirror_ground_lamp,
    /* 0x6048 */
    ID6048_Left_Mirror_selection_indicator,
    /* 0x6049 */
    ID6049_Right_Mirror_selection_indicator,

    /* 0xFDA0 */
    IDFDA0_FL_High_beam,
    IDFDA0_L_Daytime_running_lamp,
    IDFDA0_FR_Fog_lamp,
    IDFDA0_Rear_Fog_lamp,
    IDFDA0_Trunk_release_EN_C,

    IDFDA0_Mirror_fold_EN_C,
    IDFDA0_Position_lamp_Backlight,
    IDFDA0_FL_Low_beam,
    IDFDA0_FL_Fog_lamp,
    IDFDA0_Battery_Saver,
    IDFDA0_Drive_Door_Motor_Fuel_lid_unLockCtrl_EN_C,

    IDFDA0_Mirror_unfold_EN_C,
    IDFDA0_Electric_child_unLock_EN_C,
    IDFDA0_FR_RL_RR_Door_Motor_UnlockCtrl_EN_C,
    IDFDA0_ALL_Door_Motor_Lock_Ctrl_EN_C,
    IDFDA0_Right_mirror_Common_port,
    IDFDA0_Right_mirror_Horizontal_adjustment,
    IDFDA0_Left_mirror_Horizontal_adjustment,
    IDFDA0_Right_mirror_Vertical_adjustment,

    IDFDA0_Left_mirror_Vertical_adjustment,
    IDFDA0_Left_mirror_Common_port,
    IDFDA0_Right_Mirror_selection_indicator,
    IDFDA0_Left_Mirror_selection_indicator,
    IDFDA0_Turn_Light_enable,
    IDFDA0_Position_lamp_enable,
    IDFDA0_Lock_status_indication,
    /* 0xFDA1 */
    IDFDA1_Passenger_inhibit_Electric_child_protection_indicator,
    IDFDA1_IGN_RELAY_CTL,
    IDFDA1_Rear_Wiper,
    IDFDA1_Rear_Washer,
    IDFDA1_Horn,
    IDFDA1_FrontWasher,
    IDFDA1_Front_Windscreen_Heating1,
    IDFDA1_ST_RELAY_CTL,

    IDFDA1_ACC_RELAY_CTL,
    IDFDA1_Front_Windscreen_Heating2,
    IDFDA1_Front_Wiper_Low,
    IDFDA1_Rear_Windscreen_Mirror_Heating,
    IDFDA1_Front_Wiper_High,
    IDFDA1_SSB_Backlight,

    IDFDA1_Hazard_indicator,
    IDFDA1_SSB_Green_LED,
    IDFDA1_SSB_Orange_LED,
    IDFDA1_Steering_Wheel_Heating,
    IDFDA1_Steering_wheel_heating_indicator,
    IDFDA1_Nozzle_heating,

    IDFDA1_Right_turn_lamp,
    IDFDA1_Decorative_lamp,
    IDFDA1_LOGO_lamp,
    IDFDA1_Left_turn_lamp,
    /* 0xFDA2 */
    IDFDA2_Siren,
    IDFDA2_L_Cornering_Lamp,
    IDFDA2_R_Cornering_Lamp,
    IDFDA2_FL_Window_Up_Ctrl,

    IDFDA2_RR_Window_Up_Ctrl,
    IDFDA2_RR_Window_Down_Ctrl,
    IDFDA2_FL_Window_Down_Ctrl,
    IDFDA2_Position_lamp_License_lamp,
    IDFDA2_FR_High_beam,
    IDFDA2_Brake_lamp_RL_RR,
    IDFDA2_Reversing_lamp,
    IDFDA2_Welcome_lamp_RL,

    IDFDA2_Backlight_adjustment,
    IDFDA2_Trunk_Lamp,
    IDFDA2_Headlamp_height_adjustment,
    IDFDA2_FR_Low_beam,
    IDFDA2_R_Daytime_running_lamp,
    IDFDA2_Welcome_lamp_FL,
    IDFDA2_Welcome_lamp_RR,
    IDFDA2_Welcome_lamp_FR,

    IDFDA2_Interrior_Lamps,
    IDFDA2_Rearview_mirror_ground_lamp,
    IDFDA2_HIGH_Brake_lamp,
    IDFDA2_FR_Window_Up_Ctrl,
    IDFDA2_RL_Window_Up_Ctrl,
    IDFDA2_RL_Window_Down_Ctrl,
    IDFDA2_FR_Window_Down_Ctrl,

    /* END */
    IOCBI_IO_MAX,
}IOCBI_IOHandleType;

typedef enum //<----------Need to config
{
    IOCBI_DID_600D = 0,
	IOCBI_DID_600E,
	IOCBI_DID_600F,
	IOCBI_DID_6010,
	IOCBI_DID_6011,
	IOCBI_DID_6012,
	IOCBI_DID_6013,
    IOCBI_DID_6015,
    IOCBI_DID_6016,
    IOCBI_DID_6017,
    // IOCBI_DID_6018,
    IOCBI_DID_6019,
    IOCBI_DID_601A,
    IOCBI_DID_601B,
    IOCBI_DID_601C,
    IOCBI_DID_601D,
    IOCBI_DID_601E,
    IOCBI_DID_601F,
    IOCBI_DID_6021,
    IOCBI_DID_6022,
    IOCBI_DID_6023,
    IOCBI_DID_6024,
    IOCBI_DID_6025,
    IOCBI_DID_6027,
    IOCBI_DID_6028,
    IOCBI_DID_602A,
    IOCBI_DID_602B,
    IOCBI_DID_6031,
    IOCBI_DID_6032,
    IOCBI_DID_6033,
    IOCBI_DID_6034,
    IOCBI_DID_6035,
    // IOCBI_DID_6036,
    // IOCBI_DID_6037,
    IOCBI_DID_6038,
    IOCBI_DID_603B,
    IOCBI_DID_603C,
    IOCBI_DID_603D,
    IOCBI_DID_6045,
    IOCBI_DID_6046,
    IOCBI_DID_6047,
    IOCBI_DID_6048,
    IOCBI_DID_6049,
    /* EOL */
    IOCBI_DID_FDA0,
    IOCBI_DID_FDA1,
    IOCBI_DID_FDA2,
    IOCBI_DID_MAX
}IOCBI_DIDIndexType;

typedef enum
{
    IOCBI_BITMAP,
    IOCBI_NOBITMAP,
    IOCBI_BITMAP_NOBIMAP
}IOCBI_DIDCtrlType;

typedef struct
{
   U8 startBit;  /*MSB,The starting position is high.*/
   U8 length;
}IOCBI_DataPosType;

typedef struct
{
   IOCBI_DIDIndexType DIDIndex;
   IOCBI_DataPosType postion; 
   U16 safeTime;
   U32* prtSafeTimeCnt;
   U16 SOHhandle;
}IOCBI_IOCfgType;

typedef struct
{
   U16 DID;
   U8 State_Mask_len;
   IOCBI_DIDCtrlType didCtrlType;
   U8* dataBuf;
}IOCBI_DIDCfgType; //map did to buffer

typedef struct
{
   U16 SOHhandle;
   U8 *ControlStatus;
   U8 *ControlValue;
}IOCBI_SignalType; //map to soh handle

/******************************************************************************/
/*  API Functions Declare Section                                             */
/******************************************************************************/
extern void IOCBI_Init(void);
extern void IOCBI_TimingRunnable(void);
extern U16 IOCBI_Handle_Function(U8 identifier, U8 control_type,const volatile U8 * state_record,  
								const volatile U32 mask_record,U8 * response);
extern U32 IOCBI_GetSignalCtrl(IOCBI_IOHandleType ioHandle, U32 ecuCtrl);
extern U8  IOCBI_GetControlStatus(IOCBI_IOHandleType ioHandle);
extern void IOCBI_ReturnAllControlToECU(void);
extern void IOCBI_WriteChannel (U16 channelID, U16 Level, U16 ChannelType);


#endif /* IOCBI_H */

/*---- End of Module ---------------------------------------------------------*/
