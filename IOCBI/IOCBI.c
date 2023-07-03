/************************Copyright by ofilm 2020-2030*****************************
 * @file    IOCBI.c
 * @author  Application Team
 * @version V1.0.0
 * @date    8-6-2023
 * @brief   IOCBI Module
 *******************************************************************************/
#include "IOCBI.h"
#include "SOH.h"
#include "cm7_0_mcu_server.h"

static U16 IOCBI_NonBitMappedHandle(U8 didIndex, U8 control_type, const volatile U8 *state_record);
static U16 IOCBI_BitMappedHandle(U8 didIndex, U8 control_type, const volatile U8 *state_record, const volatile U32 mask);
static void Std_DataCopy(const volatile U8 *src, U8 *tgt, U16 dataLen);

static U8 iocbiControlStatus[IOCBI_IO_MAX];			// iocbi status record by IOHandle
static U8 ctrlParameterbuf1[IOCBI_DID_MAX][4];		// iocbi control value by DID
static U32 iocbiSafeTimeCnt[63]; 					// timing count record buff

static U8 s_SOHControlStatus[SOH_ALL_CHANNEL_MAX];  // iocbi status record by SOH
static U8 s_SOHControlValue[SOH_ALL_CHANNEL_MAX];	// iocbi control value record by SOH

const IOCBI_DIDCfgType IOCBI_DIDCfg[IOCBI_DID_MAX] =
{
	{0x600D,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[0]},
    {0x600E,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[1]},
    {0x600F,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[2]},
    {0x6010,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[3]},
    {0x6011,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[4]},
    {0x6012,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[5]},
    {0x6013,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[6]},
	{0x6015,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[7]},
	{0x6016,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[8]},
	{0x6017,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[9]},
	{0x6019,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[10]},
	{0x601A,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[11]},
	{0x601B,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[12]},
	{0x601C,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[13]},
	{0x601D,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[14]},
	{0x601E,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[15]},
	{0x601F,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[16]},
	{0x6021,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[17]},
	{0x6022,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[18]},
	{0x6023,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[19]},
	{0x6024,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[20]},
	{0x6025,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[21]},
	{0x6027,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[22]},
	{0x6028,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[23]},
	{0x602A,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[24]},
	{0x602B,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[25]},
	{0x6031,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[26]},
	{0x6032,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[27]},
	{0x6033,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[28]},
	{0x6034,      1, 	IOCBI_NOBITMAP, 	(U8*)ctrlParameterbuf1[29]}, // PWM Control
	{0x6035,      1, 	IOCBI_NOBITMAP, 	(U8*)ctrlParameterbuf1[30]}, // PWM Percent Control
	{0x6038,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[31]},
	{0x603B,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[32]},
	{0x603C,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[33]},
	{0x603D,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[34]},
	{0x6045,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[35]},
	{0x6046,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[36]},
	{0x6047,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[37]},
	{0x6048,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[38]},
	{0x6049,      1, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[39]},
	/* EOL */
	{0xFDA0,      4, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[40]},
	{0xFDA1,      4, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[41]},
	{0xFDA2,      4, 	IOCBI_BITMAP,   	(U8*)ctrlParameterbuf1[42]},
};

const IOCBI_IOCfgType IOCBI_IOCfg[IOCBI_IO_MAX] = 
{
    {IOCBI_DID_600D, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[0], SOH_ALL_DOHS15_Left_turn_lamp},
    
    {IOCBI_DID_600E, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[1], SOH_ALL_DOHS09_Right_turn_lamp},

    {IOCBI_DID_600F, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[2], SOH_ALL_DOWHS04_Position_lamp_Backlight},
    {IOCBI_DID_600F, {1, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[3], SOH_ALL_DOWHS16_Position_lamp_License_lamp},

    {IOCBI_DID_6010, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[4], SOH_ALL_DOWHS05_FL_Low_beam},
    {IOCBI_DID_6010, {1, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[5], SOH_ALL_DOWHS21_FR_Low_beam},

    {IOCBI_DID_6011, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[6], SOH_ALL_DOWHS00_FL_High_beam},
    {IOCBI_DID_6011, {1, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[7], SOH_ALL_DOWHS17_FR_High_beam},

    {IOCBI_DID_6012, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[8], SOH_ALL_DOHS04_FL_Fog_lamp},
    {IOCBI_DID_6012, {1, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[9], SOH_ALL_DOHS02_FR_Fog_lamp},

    {IOCBI_DID_6013, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[10], SOH_ALL_DOHS03_Rear_Fog_lamp},

    {IOCBI_DID_6015, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[11], SOH_ALL_DOHS01_L_Daytime_running_lamp},
    {IOCBI_DID_6015, {1, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[12], SOH_ALL_DOHS24_R_Daytime_running_lamp},

    {IOCBI_DID_6016, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[13], SOH_ALL_DOWLS01_Interrior_Lamps},  // PWM

    {IOCBI_DID_6017, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[14], SOH_ALL_DOHS22_Welcome_lamp_RL},
    {IOCBI_DID_6017, {1, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[15], SOH_ALL_DOHS25_Welcome_lamp_FL},
    {IOCBI_DID_6017, {2, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[16], SOH_ALL_DOHS26_Welcome_lamp_RR},
    {IOCBI_DID_6017, {3, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[17], SOH_ALL_DOHS27_Welcome_lamp_FR},

    {IOCBI_DID_6019, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[18], SOH_ALL_DOHS35_Hazard_indicator},	 // 400ms flick
    {IOCBI_DID_6019, {1, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[19], SOH_ALL_DOHS15_Left_turn_lamp},
    {IOCBI_DID_6019, {2, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[20], SOH_ALL_DOHS09_Right_turn_lamp},

    {IOCBI_DID_601A, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[21], SOH_ALL_DOWHS18_Brake_lamp_RL_RR},
    {IOCBI_DID_601A, {1, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[22], SOH_ALL_DOWHS22_HIGH_Brake_lamp},

    {IOCBI_DID_601B, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[23], SOH_ALL_DOLS07_Front_Wiper_Low},
    {IOCBI_DID_601B, {1, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[24], SOH_ALL_DOLS09_Front_Wiper_High},

    {IOCBI_DID_601C, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[25], SOH_ALL_DOLS04_FrontWasher},

    {IOCBI_DID_601D, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[26], SOH_ALL_DOLS07_Front_Wiper_Low},

    {IOCBI_DID_601E, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[27], SOH_ALL_OR09_FL_Window_Up_Ctrl}, // window
    {IOCBI_DID_601E, {1, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[28], SOH_ALL_OR12_FL_Window_Down_Ctrl},

    {IOCBI_DID_601F, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[29], SOH_ALL_OR13_FR_Window_Up_Ctrl}, // window
    {IOCBI_DID_601F, {1, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[30], SOH_ALL_OR16_FR_Window_Down_Ctrl},

    {IOCBI_DID_6021, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[31], SOH_ALL_OR14_RL_Window_Up_Ctrl}, // window
    {IOCBI_DID_6021, {1, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[32], SOH_ALL_OR15_RL_Window_Down_Ctrl},

    {IOCBI_DID_6022, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[33], SOH_ALL_OR10_RR_Window_Up_Ctrl}, // window
    {IOCBI_DID_6022, {1, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[34], SOH_ALL_OR11_RR_Window_Down_Ctrl},

    {IOCBI_DID_6023, {0, 1}, IOCBI_NO_SAFE_TIME,   NULL, 				  SOH_ALL_OR04_Drive_Door_Motor_Fuel_lid_unLockCtrl_EN_C}, // trigger once
    {IOCBI_DID_6023, {1, 1}, IOCBI_NO_SAFE_TIME,   NULL, 				  SOH_ALL_OR07_FR_RL_RR_Door_Motor_UnlockCtrl_EN_C},
    {IOCBI_DID_6023, {2, 1}, IOCBI_NO_SAFE_TIME,   NULL, 				  SOH_ALL_OR08_ALL_Door_Motor_Lock_Ctrl_EN_C},
    {IOCBI_DID_6023, {3, 1}, IOCBI_NO_SAFE_TIME,   NULL, 				  SOH_ALL_OR06_Electric_child_unLock_EN_C},

    {IOCBI_DID_6024, {0, 1}, IOCBI_NO_SAFE_TIME,   NULL, 				  SOH_ALL_OR02_Trunk_release_EN_C},  // trigger once

    {IOCBI_DID_6025, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[40], SOH_ALL_DOLS03_Horn}, // tigger horn or siren by setting: 0x00 horn 0x01 siren
    {IOCBI_DID_6025, {1, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[41], SOH_ALL_DOHS16_Siren},

    {IOCBI_DID_6027, {0, 1}, IOCBI_NO_SAFE_TIME,   NULL, 				  SOH_ALL_OR03_Mirror_fold_EN_C}, // tigger once
    {IOCBI_DID_6027, {1, 1}, IOCBI_NO_SAFE_TIME,   NULL, 				  SOH_ALL_OR05_Mirror_unfold_EN_C},

    {IOCBI_DID_6028, {0, 1}, IOCBI_SAFE_TIME_60SEC, &iocbiSafeTimeCnt[44], SOH_ALL_DOLS08_Rear_Windscreen_Mirror_Heating},

    {IOCBI_DID_602A, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[45], SOH_ALL_DOHS32_Passenger_inhibit_Electric_child_protection_indicator},

    {IOCBI_DID_602B, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[46], SOH_ALL_DOHS23_Trunk_Lamp},

    {IOCBI_DID_6031, {0, 1}, IOCBI_SAFE_TIME_60SEC, &iocbiSafeTimeCnt[47], SOH_ALL_DOLS05_Front_Windscreen_Heating1},
    {IOCBI_DID_6031, {1, 1}, IOCBI_SAFE_TIME_60SEC, &iocbiSafeTimeCnt[48], SOH_ALL_DOLS06_Front_Windscreen_Heating2},

    {IOCBI_DID_6032, {0, 1}, IOCBI_SAFE_TIME_60SEC, &iocbiSafeTimeCnt[49], SOH_ALL_DOLS10_Steering_Wheel_Heating},
    {IOCBI_DID_6032, {1, 1}, IOCBI_SAFE_TIME_60SEC, &iocbiSafeTimeCnt[50], SOH_ALL_DOHS40_Steering_wheel_heating_indicator},

    {IOCBI_DID_6033, {0, 1}, IOCBI_SAFE_TIME_60SEC, &iocbiSafeTimeCnt[51], SOH_ALL_DOHS08_Nozzle_heating},

    {IOCBI_DID_6034, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[52], SOH_ALL_DOWHS20_Headlamp_height_adjustment}, // PWM

    {IOCBI_DID_6035, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[53], SOH_ALL_DOWHS19_Backlight_adjustment}, // PWM

    {IOCBI_DID_6038, {0, 1}, IOCBI_NO_SAFE_TIME,   &iocbiSafeTimeCnt[54], SOH_ALL_OR06_Electric_child_unLock_EN_C},

    {IOCBI_DID_603B, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[55], SOH_ALL_DOHS04_FL_Fog_lamp},

    {IOCBI_DID_603C, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[56], SOH_ALL_DOHS02_FR_Fog_lamp},

    {IOCBI_DID_603D, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[57], SOH_ALL_DOHS31_Lock_status_indication},

    {IOCBI_DID_6045, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[58], SOH_ALL_DOHS17_L_Cornering_Lamp},

    {IOCBI_DID_6046, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[59], SOH_ALL_DOHS18_R_Cornering_Lamp},

    {IOCBI_DID_6047, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[60], SOH_ALL_DOHS28_Rearview_mirror_ground_lamp},

    {IOCBI_DID_6048, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[61], SOH_ALL_DOHS30_Left_Mirror_selection_indicator},

    {IOCBI_DID_6049, {0, 1}, IOCBI_SAFE_TIME_6SEC, &iocbiSafeTimeCnt[62], SOH_ALL_DOHS29_Right_Mirror_selection_indicator},

	/* EOL Field */
	/* 0xFDA0 */
	/* byte 0 */
	{IOCBI_DID_FDA0, {0, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWHS00_FL_High_beam},
	{IOCBI_DID_FDA0, {1, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS01_L_Daytime_running_lamp},
	{IOCBI_DID_FDA0, {3, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS02_FR_Fog_lamp},
	{IOCBI_DID_FDA0, {4, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS03_Rear_Fog_lamp},
	{IOCBI_DID_FDA0, {7, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR02_Trunk_release_EN_C},
	/* byte 1 */
	{IOCBI_DID_FDA0, {8, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR03_Mirror_fold_EN_C},
	{IOCBI_DID_FDA0, {9, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWHS04_Position_lamp_Backlight},
	{IOCBI_DID_FDA0, {10, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWHS05_FL_Low_beam},
	{IOCBI_DID_FDA0, {12, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS04_FL_Fog_lamp},
	{IOCBI_DID_FDA0, {13, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWHS07_Battery_Saver},
	{IOCBI_DID_FDA0, {15, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR04_Drive_Door_Motor_Fuel_lid_unLockCtrl_EN_C},
	/* byte 2 */
	{IOCBI_DID_FDA0, {16, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR05_Mirror_unfold_EN_C},
	{IOCBI_DID_FDA0, {17, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR06_Electric_child_unLock_EN_C},
	{IOCBI_DID_FDA0, {18, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR07_FR_RL_RR_Door_Motor_UnlockCtrl_EN_C},
	{IOCBI_DID_FDA0, {19, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR08_ALL_Door_Motor_Lock_Ctrl_EN_C},
	{IOCBI_DID_FDA0, {20, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHB01_Right_mirror_Common_port},
	{IOCBI_DID_FDA0, {21, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHB02_Right_mirror_Horizontal_adjustment},
	{IOCBI_DID_FDA0, {22, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHB03_Left_mirror_Horizontal_adjustment},
	{IOCBI_DID_FDA0, {23, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHB04_Right_mirror_Vertical_adjustment},
	/* byte 3 */
	{IOCBI_DID_FDA0, {24, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHB05_Left_mirror_Vertical_adjustment},
	{IOCBI_DID_FDA0, {25, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHB06_Left_mirror_Common_port},
	{IOCBI_DID_FDA0, {26, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS29_Right_Mirror_selection_indicator},
	{IOCBI_DID_FDA0, {27, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS30_Left_Mirror_selection_indicator},
	{IOCBI_DID_FDA0, {28, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWHS09_Turn_Light_enable},
	{IOCBI_DID_FDA0, {29, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWHS10_Position_lamp_enable},
	{IOCBI_DID_FDA0, {31, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS31_Lock_status_indication},
	/* 0xFDA1 */
	/* byte 0 */
	{IOCBI_DID_FDA1, {0, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS32_Passenger_inhibit_Electric_child_protection_indicator},
	{IOCBI_DID_FDA1, {1, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS05_IGN_RELAY_CTL},
	{IOCBI_DID_FDA1, {2, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOLS01_Rear_Wiper},
	{IOCBI_DID_FDA1, {3, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOLS02_Rear_Washer},
	{IOCBI_DID_FDA1, {4, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOLS03_Horn},
	{IOCBI_DID_FDA1, {5, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOLS04_FrontWasher},
	{IOCBI_DID_FDA1, {6, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOLS05_Front_Windscreen_Heating1},
	{IOCBI_DID_FDA1, {7, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS06_ST_RELAY_CTL},
	/* byte 1 */
	{IOCBI_DID_FDA1, {8, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS07_ACC_RELAY_CTL},
	{IOCBI_DID_FDA1, {9, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOLS06_Front_Windscreen_Heating2},
	{IOCBI_DID_FDA1, {10, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOLS07_Front_Wiper_Low},
	{IOCBI_DID_FDA1, {11, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOLS08_Rear_Windscreen_Mirror_Heating},
	{IOCBI_DID_FDA1, {12, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOLS09_Front_Wiper_High},
	{IOCBI_DID_FDA1, {14, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWHS12_SSB_Backlight},
	/* byte 2 */
	{IOCBI_DID_FDA1, {16, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS35_Hazard_indicator},
	{IOCBI_DID_FDA1, {19, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS38_SSB_Green_LED},
	{IOCBI_DID_FDA1, {20, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS39_SSB_Orange_LED},
	{IOCBI_DID_FDA1, {21, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOLS10_Steering_Wheel_Heating},
	{IOCBI_DID_FDA1, {22, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS40_Steering_wheel_heating_indicator},
	{IOCBI_DID_FDA1, {23, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS08_Nozzle_heating},
	/* byte 3 */
	{IOCBI_DID_FDA1, {24, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS09_Right_turn_lamp},
	{IOCBI_DID_FDA1, {25, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS10_Decorative_lamp},
	{IOCBI_DID_FDA1, {26, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS11_LOGO_lamp},
	{IOCBI_DID_FDA1, {31, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS15_Left_turn_lamp},
	/* 0xFDA2 */
	/* byte 0 */
	{IOCBI_DID_FDA2, {0, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS16_Siren},
	{IOCBI_DID_FDA2, {1, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS17_L_Cornering_Lamp},
	{IOCBI_DID_FDA2, {2, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS18_R_Cornering_Lamp},
	{IOCBI_DID_FDA2, {7, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR09_FL_Window_Up_Ctrl},
	/* byte 1 */
	{IOCBI_DID_FDA2, {8, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR10_RR_Window_Up_Ctrl},
	{IOCBI_DID_FDA2, {9, 1},  IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR11_RR_Window_Down_Ctrl},
	{IOCBI_DID_FDA2, {10, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR12_FL_Window_Down_Ctrl},
	{IOCBI_DID_FDA2, {11, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWHS16_Position_lamp_License_lamp},
	{IOCBI_DID_FDA2, {12, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWHS17_FR_High_beam},
	{IOCBI_DID_FDA2, {13, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWHS18_Brake_lamp_RL_RR},
	{IOCBI_DID_FDA2, {14, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS21_Reversing_lamp},
	{IOCBI_DID_FDA2, {15, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS22_Welcome_lamp_RL},
	/* byte 2 */
	{IOCBI_DID_FDA2, {16, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWHS19_Backlight_adjustment},
	{IOCBI_DID_FDA2, {17, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS23_Trunk_Lamp},
	{IOCBI_DID_FDA2, {18, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWHS20_Headlamp_height_adjustment},
	{IOCBI_DID_FDA2, {19, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWHS21_FR_Low_beam},
	{IOCBI_DID_FDA2, {20, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS24_R_Daytime_running_lamp},
	{IOCBI_DID_FDA2, {21, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS25_Welcome_lamp_FL},
	{IOCBI_DID_FDA2, {22, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS26_Welcome_lamp_RR},
	{IOCBI_DID_FDA2, {23, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS27_Welcome_lamp_FR},
	/* byte 3 */
	{IOCBI_DID_FDA2, {24, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWLS01_Interrior_Lamps},
	{IOCBI_DID_FDA2, {25, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOHS28_Rearview_mirror_ground_lamp},
	{IOCBI_DID_FDA2, {26, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_DOWHS22_HIGH_Brake_lamp},
	{IOCBI_DID_FDA2, {28, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR13_FR_Window_Up_Ctrl},
	{IOCBI_DID_FDA2, {29, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR14_RL_Window_Up_Ctrl},
	{IOCBI_DID_FDA2, {30, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR15_RL_Window_Down_Ctrl},
	{IOCBI_DID_FDA2, {31, 1}, IOCBI_NO_SAFE_TIME, NULL, SOH_ALL_OR16_FR_Window_Down_Ctrl},
};

const IOCBI_SignalType IOCBI_SignalToSOH[] =
{  
    {SOH_ALL_DOHS32_Passenger_inhibit_Electric_child_protection_indicator,  &s_SOHControlStatus[0], 	&s_SOHControlValue[0]},
	{SOH_ALL_DOHS31_Lock_status_indication,									&s_SOHControlStatus[1], 	&s_SOHControlValue[1]},
	{SOH_ALL_DOWHS09_Turn_Light_enable,										&s_SOHControlStatus[2], 	&s_SOHControlValue[2]},
	{SOH_ALL_DOHS29_Right_Mirror_selection_indicator,						&s_SOHControlStatus[3], 	&s_SOHControlValue[3]},
	{SOH_ALL_DOHS30_Left_Mirror_selection_indicator,						&s_SOHControlStatus[4], 	&s_SOHControlValue[4]},
	{SOH_ALL_DOWHS10_Position_lamp_enable,									&s_SOHControlStatus[5], 	&s_SOHControlValue[5]},
	{SOH_ALL_DOHB03_Left_mirror_Horizontal_adjustment,						&s_SOHControlStatus[6], 	&s_SOHControlValue[6]},
	{SOH_ALL_DOHB06_Left_mirror_Common_port,								&s_SOHControlStatus[7], 	&s_SOHControlValue[7]},
	{SOH_ALL_DOHB04_Right_mirror_Vertical_adjustment,						&s_SOHControlStatus[8], 	&s_SOHControlValue[8]},
	{SOH_ALL_DOHB01_Right_mirror_Common_port,								&s_SOHControlStatus[9], 	&s_SOHControlValue[9]},
	{SOH_ALL_DOHB05_Left_mirror_Vertical_adjustment,						&s_SOHControlStatus[10], 	&s_SOHControlValue[10]},
	{SOH_ALL_DOHB02_Right_mirror_Horizontal_adjustment,						&s_SOHControlStatus[11], 	&s_SOHControlValue[11]},
	{SOH_ALL_DOHS40_Steering_wheel_heating_indicator,						&s_SOHControlStatus[12], 	&s_SOHControlValue[12]},
	{SOH_ALL_DOHS34_Start_Stop_indication,									&s_SOHControlStatus[13], 	&s_SOHControlValue[13]},
	{SOH_ALL_DOHS38_SSB_Green_LED,											&s_SOHControlStatus[14], 	&s_SOHControlValue[14]},
	{SOH_ALL_DOWHS12_SSB_Backlight,											&s_SOHControlStatus[15], 	&s_SOHControlValue[15]},
	{SOH_ALL_DOHS35_Hazard_indicator,										&s_SOHControlStatus[16], 	&s_SOHControlValue[16]},
	{SOH_ALL_DOHS39_SSB_Orange_LED,											&s_SOHControlStatus[17], 	&s_SOHControlValue[17]},
	{SOH_ALL_DOHS15_Left_turn_lamp,											&s_SOHControlStatus[18], 	&s_SOHControlValue[18]},
	{SOH_ALL_DOHS09_Right_turn_lamp,										&s_SOHControlStatus[19], 	&s_SOHControlValue[19]},
	{SOH_ALL_DOHS16_Siren,													&s_SOHControlStatus[20], 	&s_SOHControlValue[20]},
	{SOH_ALL_DOHS08_Nozzle_heating,											&s_SOHControlStatus[21], 	&s_SOHControlValue[21]},
	{SOH_ALL_DOWHS05_FL_Low_beam,											&s_SOHControlStatus[22], 	&s_SOHControlValue[22]},
	{SOH_ALL_DOWHS00_FL_High_beam,											&s_SOHControlStatus[23], 	&s_SOHControlValue[23]},
	{SOH_ALL_DOHS01_L_Daytime_running_lamp,									&s_SOHControlStatus[24], 	&s_SOHControlValue[24]},
	{SOH_ALL_DOWHS04_Position_lamp_Backlight,								&s_SOHControlStatus[25], 	&s_SOHControlValue[25]},
	{SOH_ALL_DOWHS21_FR_Low_beam,											&s_SOHControlStatus[26], 	&s_SOHControlValue[26]},
	{SOH_ALL_DOWHS17_FR_High_beam,											&s_SOHControlStatus[27], 	&s_SOHControlValue[27]},
	{SOH_ALL_DOHS24_R_Daytime_running_lamp,									&s_SOHControlStatus[28], 	&s_SOHControlValue[28]},
	{SOH_ALL_DOWHS16_Position_lamp_License_lamp,							&s_SOHControlStatus[29], 	&s_SOHControlValue[29]},
	{SOH_ALL_DOHS14_HSD_Output3,											&s_SOHControlStatus[30], 	&s_SOHControlValue[30]},
	{SOH_ALL_DOWHS13_Atmosphere_lamp_R,										&s_SOHControlStatus[31], 	&s_SOHControlValue[31]},
	{SOH_ALL_DOHS13_Side_marker_lamp,										&s_SOHControlStatus[32], 	&s_SOHControlValue[32]},
	{SOH_ALL_DOWHS14_Atmosphere_lamp_G,										&s_SOHControlStatus[33], 	&s_SOHControlValue[33]},
	{SOH_ALL_DOWHS15_Atmosphere_lamp_B,										&s_SOHControlStatus[34], 	&s_SOHControlValue[34]},
	{SOH_ALL_DOHS20_Grille_lamp,											&s_SOHControlStatus[35], 	&s_SOHControlValue[35]},
	{SOH_ALL_DOHS21_Reversing_lamp,											&s_SOHControlStatus[36], 	&s_SOHControlValue[36]},
	{SOH_ALL_DOWHS18_Brake_lamp_RL_RR,										&s_SOHControlStatus[37], 	&s_SOHControlValue[37]},
	{SOH_ALL_DOWHS02_HSD_Output1,											&s_SOHControlStatus[38], 	&s_SOHControlValue[38]},
	{SOH_ALL_DOWHS06_HSD_Output2,											&s_SOHControlStatus[39], 	&s_SOHControlValue[39]},
	{SOH_ALL_DOHS17_L_Cornering_Lamp,										&s_SOHControlStatus[40], 	&s_SOHControlValue[40]},
	{SOH_ALL_DOHS18_R_Cornering_Lamp,										&s_SOHControlStatus[41], 	&s_SOHControlValue[41]},
	{SOH_ALL_DOHS19_Turn_lamp_2,											&s_SOHControlStatus[42], 	&s_SOHControlValue[42]},
	{SOH_ALL_DOHS12_Turn_lamp_1,											&s_SOHControlStatus[43], 	&s_SOHControlValue[43]},
	{SOH_ALL_DOHS11_LOGO_lamp,												&s_SOHControlStatus[44], 	&s_SOHControlValue[44]},
	{SOH_ALL_DOHS10_Decorative_lamp,										&s_SOHControlStatus[45], 	&s_SOHControlValue[45]},
	{SOH_ALL_DOHS03_Rear_Fog_lamp,											&s_SOHControlStatus[46], 	&s_SOHControlValue[46]},
	{SOH_ALL_DOWHS07_Battery_Saver,											&s_SOHControlStatus[47], 	&s_SOHControlValue[47]},
	{SOH_ALL_DOHS04_FL_Fog_lamp,											&s_SOHControlStatus[48], 	&s_SOHControlValue[48]},
	{SOH_ALL_DOHS02_FR_Fog_lamp,											&s_SOHControlStatus[49], 	&s_SOHControlValue[49]},
	{SOH_ALL_DOWHS08_Brake_lamp_2,											&s_SOHControlStatus[50], 	&s_SOHControlValue[50]},
	{SOH_ALL_DOWHS03_Foot_lamp,												&s_SOHControlStatus[51], 	&s_SOHControlValue[51]},
	{SOH_ALL_DOWHS11_HSD_BJT_Output1,										&s_SOHControlStatus[52], 	&s_SOHControlValue[52]},
	{SOH_ALL_DOHS05_IGN_RELAY_CTL,											&s_SOHControlStatus[53], 	&s_SOHControlValue[53]},
	{SOH_ALL_DOHS06_ST_RELAY_CTL,											&s_SOHControlStatus[54], 	&s_SOHControlValue[54]},
	{SOH_ALL_DOHS07_ACC_RELAY_CTL,											&s_SOHControlStatus[55], 	&s_SOHControlValue[55]},
	{SOH_ALL_DOWHS19_Backlight_adjustment,									&s_SOHControlStatus[56], 	&s_SOHControlValue[56]},
	{SOH_ALL_DOHS28_Rearview_mirror_ground_lamp,							&s_SOHControlStatus[57], 	&s_SOHControlValue[57]},
	{SOH_ALL_DOHS23_Trunk_Lamp,												&s_SOHControlStatus[58], 	&s_SOHControlValue[58]},
	{SOH_ALL_DOWHS22_HIGH_Brake_lamp,										&s_SOHControlStatus[59], 	&s_SOHControlValue[59]},
	{SOH_ALL_DOHS26_Welcome_lamp_RR,										&s_SOHControlStatus[60], 	&s_SOHControlValue[60]},
	{SOH_ALL_DOHS25_Welcome_lamp_FL,										&s_SOHControlStatus[61], 	&s_SOHControlValue[61]},
	{SOH_ALL_DOHS22_Welcome_lamp_RL,										&s_SOHControlStatus[62], 	&s_SOHControlValue[62]},
	{SOH_ALL_DOHS27_Welcome_lamp_FR,										&s_SOHControlStatus[63], 	&s_SOHControlValue[63]},
	{SOH_ALL_DOLS10_Steering_Wheel_Heating,									&s_SOHControlStatus[64], 	&s_SOHControlValue[64]},
	{SOH_ALL_DOLS03_Horn,													&s_SOHControlStatus[65], 	&s_SOHControlValue[65]},
	{SOH_ALL_DOLS02_Rear_Washer,											&s_SOHControlStatus[66], 	&s_SOHControlValue[66]},
	{SOH_ALL_DOLS05_Front_Windscreen_Heating1,								&s_SOHControlStatus[67], 	&s_SOHControlValue[67]},
	{SOH_ALL_DOLS06_Front_Windscreen_Heating2,								&s_SOHControlStatus[68], 	&s_SOHControlValue[68]},
	{SOH_ALL_DOLS08_Rear_Windscreen_Mirror_Heating,							&s_SOHControlStatus[69], 	&s_SOHControlValue[69]},
	{SOH_ALL_DOLS09_Front_Wiper_High,										&s_SOHControlStatus[70], 	&s_SOHControlValue[70]},
	{SOH_ALL_DOLS04_FrontWasher,											&s_SOHControlStatus[71], 	&s_SOHControlValue[71]},
	{SOH_ALL_DOHS37_HSD_BJT_Output4_EN,										&s_SOHControlStatus[72], 	&s_SOHControlValue[72]},
	{SOH_ALL_DOHS33_HSD_BJT_Output3_EN,										&s_SOHControlStatus[73], 	&s_SOHControlValue[73]},
	{SOH_ALL_DOHS36_HSD_BJT_Output2_EN,										&s_SOHControlStatus[74], 	&s_SOHControlValue[74]},
	{SOH_ALL_OR01_Relay_EN_C,												&s_SOHControlStatus[75], 	&s_SOHControlValue[75]},
	{SOH_ALL_OR06_Electric_child_unLock_EN_C,								&s_SOHControlStatus[76], 	&s_SOHControlValue[76]},
	{SOH_ALL_OR04_Drive_Door_Motor_Fuel_lid_unLockCtrl_EN_C,				&s_SOHControlStatus[77], 	&s_SOHControlValue[77]},
	{SOH_ALL_OR08_ALL_Door_Motor_Lock_Ctrl_EN_C,							&s_SOHControlStatus[78], 	&s_SOHControlValue[78]},
	{SOH_ALL_OR07_FR_RL_RR_Door_Motor_UnlockCtrl_EN_C,						&s_SOHControlStatus[79], 	&s_SOHControlValue[79]},
	{SOH_ALL_OR03_Mirror_fold_EN_C,											&s_SOHControlStatus[80], 	&s_SOHControlValue[80]},
	{SOH_ALL_OR05_Mirror_unfold_EN_C,										&s_SOHControlStatus[81], 	&s_SOHControlValue[81]},
	{SOH_ALL_OR02_Trunk_release_EN_C,										&s_SOHControlStatus[82], 	&s_SOHControlValue[82]},
	{SOH_ALL_OPH01_PUP1_EN,													&s_SOHControlStatus[83], 	&s_SOHControlValue[83]},
	{SOH_ALL_OPH02_PUP2_EN,													&s_SOHControlStatus[84], 	&s_SOHControlValue[84]},
	{SOH_ALL_OPH03_PUP3_EN,													&s_SOHControlStatus[85], 	&s_SOHControlValue[85]},
	{SOH_ALL_DOLS01_Rear_Wiper,												&s_SOHControlStatus[86], 	&s_SOHControlValue[86]},
	{SOH_ALL_DOLS07_Front_Wiper_Low,										&s_SOHControlStatus[87], 	&s_SOHControlValue[87]},
	{SOH_ALL_OR09_FL_Window_Up_Ctrl,										&s_SOHControlStatus[88], 	&s_SOHControlValue[88]},
	{SOH_ALL_OR10_RR_Window_Up_Ctrl,										&s_SOHControlStatus[89], 	&s_SOHControlValue[89]},
	{SOH_ALL_OR11_RR_Window_Down_Ctrl,										&s_SOHControlStatus[90], 	&s_SOHControlValue[90]},
	{SOH_ALL_OR12_FL_Window_Down_Ctrl,										&s_SOHControlStatus[91], 	&s_SOHControlValue[91]},
	{SOH_ALL_OR13_FR_Window_Up_Ctrl,										&s_SOHControlStatus[92], 	&s_SOHControlValue[92]},
	{SOH_ALL_OR14_RL_Window_Up_Ctrl,										&s_SOHControlStatus[93], 	&s_SOHControlValue[93]},
	{SOH_ALL_OR15_RL_Window_Down_Ctrl,										&s_SOHControlStatus[94], 	&s_SOHControlValue[94]},
	{SOH_ALL_OR16_FR_Window_Down_Ctrl,										&s_SOHControlStatus[95], 	&s_SOHControlValue[95]},
	{SOH_ALL_DO_CHANNEL_MAX,												&s_SOHControlStatus[96], 	&s_SOHControlValue[96]},
	{SOH_ALL_DOWHS20_Headlamp_height_adjustment,							&s_SOHControlStatus[97], 	&s_SOHControlValue[97]},
	{SOH_ALL_DOWLS01_Interrior_Lamps,										&s_SOHControlStatus[98], 	&s_SOHControlValue[98]},
	{SOH_ALL_CHANNEL_MAX,                          							NULL, 						NULL				  }
};

void IOCBI_Init(void)
{
	U16 ioHandle = 0x00u;
	IOCBI_IOCfgType *prtIOCfg;
	for (ioHandle = 0x00u; ioHandle < IOCBI_IO_MAX; ioHandle++)
	{
		prtIOCfg = (IOCBI_IOCfgType *)&IOCBI_IOCfg[ioHandle];
		if ((prtIOCfg->safeTime != IOCBI_NO_SAFE_TIME) && (prtIOCfg->prtSafeTimeCnt != NULL))
		{
			(*prtIOCfg->prtSafeTimeCnt) = 0x00u;
		}
		iocbiControlStatus[ioHandle] = IOCBI_RETURNCONTROLTOECU;
	}
	memset(&s_SOHControlStatus, IOCBI_RETURNCONTROLTOECU, sizeof(s_SOHControlStatus));
	memset(&s_SOHControlValue, 0, sizeof(s_SOHControlValue));
}

void IOCBI_TimingRunnable(void)
{
	U16 ioIndex = 0x00u;
	IOCBI_IOCfgType *prtIOCfg;
	U64 TargetTime = 0;
	U64 CurTime = 0;
	U32 TimeValueGet = appclock_gettick_1ms();

	for (ioIndex = 0x00u; ioIndex < IOCBI_IO_MAX; ioIndex++)
	{
		prtIOCfg = (IOCBI_IOCfgType *)&IOCBI_IOCfg[ioIndex];
		if ((prtIOCfg->safeTime != IOCBI_NO_SAFE_TIME) && (prtIOCfg->prtSafeTimeCnt != NULL))
		{
			if (IOCBI_RETURNCONTROLTOECU != iocbiControlStatus[ioIndex])
			{
				TargetTime = (U64)(*prtIOCfg->prtSafeTimeCnt) + prtIOCfg->safeTime;
				if (TimeValueGet < (*prtIOCfg->prtSafeTimeCnt))
				{
					CurTime = TimeValueGet + IOCBI_U32_MAX;
				}
				else
				{
					CurTime = TimeValueGet;
				}

				if (CurTime >= TargetTime)
				{
					(*prtIOCfg->prtSafeTimeCnt) = 0x00u;
					iocbiControlStatus[ioIndex] = IOCBI_RETURNCONTROLTOECU;
					s_SOHControlStatus[prtIOCfg->SOHhandle] = IOCBI_RETURNCONTROLTOECU;
				}
			}
		}
	}
}

U16 IOCBI_Handle_Function(U8 identifier, U8 control_type, const volatile U8 *state_record,
						  const volatile U32 mask_record, U8 *response)
{
	U16 rtn_val = S_DSM_RC_OK;
	IOCBI_DIDCfgType *prtDIDCfg = NULL;

	if (identifier >= IOCBI_DID_MAX)
	{
		rtn_val = S_DSM_RC_ROOR;
	}
	else
	{
		prtDIDCfg = (IOCBI_DIDCfgType *)&IOCBI_DIDCfg[identifier];

		if (prtDIDCfg->didCtrlType == IOCBI_BITMAP)
		{
			rtn_val = IOCBI_BitMappedHandle(identifier, control_type, state_record, mask_record);
		}
		else if (prtDIDCfg->didCtrlType == IOCBI_NOBITMAP)
		{
			rtn_val = IOCBI_NonBitMappedHandle(identifier, control_type, state_record);
		}
		else
		{

		}
	}

	return (rtn_val);
}

U32 IOCBI_GetSignalCtrl(IOCBI_IOHandleType ioHandle, U32 ecuCtrl)
{
	U32 rtn_val = 0x00u;
	U8 byteOffset = 0x00u;
	U8 bitOffset = 0x00u;
	U8 msbSpace = 0x00u;
	U8 lsbSpace = 0x00u;
	U8 msbVaule = 0x00u;
	U8 didIndex = 0x00u;
	IOCBI_IOCfgType *prtIOCfg;
	IOCBI_DIDCfgType *prtDIDCfg;

	prtIOCfg = (IOCBI_IOCfgType *)&IOCBI_IOCfg[ioHandle];
	didIndex = prtIOCfg->DIDIndex;
	prtDIDCfg = (IOCBI_DIDCfgType *)&IOCBI_DIDCfg[didIndex];

	byteOffset = prtIOCfg->postion.startBit / 8;
	bitOffset = prtIOCfg->postion.startBit % 8;
	msbSpace = 7 - bitOffset; /*Spare high*/

	if (IOCBI_RETURNCONTROLTOECU == iocbiControlStatus[ioHandle])
	{
		rtn_val = ecuCtrl;
	}
	else
	{
		if (prtIOCfg->postion.length <= 8)
		{
			lsbSpace = (bitOffset + 1) - prtIOCfg->postion.length; /*Spare low*/

			rtn_val = prtDIDCfg->dataBuf[byteOffset];
			rtn_val = (U8)(rtn_val << msbSpace);
			rtn_val = (U8)(rtn_val >> (msbSpace + lsbSpace));
		}
		else if (prtIOCfg->postion.length <= 16)
		{
			msbVaule = prtDIDCfg->dataBuf[byteOffset];
			msbVaule = (U8)(msbVaule << msbSpace);
			msbVaule = (U8)(msbVaule >> msbSpace);
			rtn_val = (U32)msbVaule << 8;
			rtn_val += prtDIDCfg->dataBuf[byteOffset + 1];
		}
		else if (prtIOCfg->postion.length <= 24)
		{
			msbVaule = prtDIDCfg->dataBuf[byteOffset];
			msbVaule = (U8)(msbVaule << msbSpace);
			msbVaule = (U8)(msbVaule >> msbSpace);
			rtn_val = (U32)msbVaule << 16;
			rtn_val += (prtDIDCfg->dataBuf[byteOffset + 1] << 8);
			rtn_val += prtDIDCfg->dataBuf[byteOffset + 2];
		}
		else
		{
			msbVaule = prtDIDCfg->dataBuf[byteOffset];
			msbVaule = (U8)(msbVaule << msbSpace);
			msbVaule = (U8)(msbVaule >> msbSpace);
			rtn_val = (U32)msbVaule << 24;
			rtn_val += (prtDIDCfg->dataBuf[byteOffset + 1] << 16);
			rtn_val += (prtDIDCfg->dataBuf[byteOffset + 2] << 8);
			rtn_val += prtDIDCfg->dataBuf[byteOffset + 3];
		}
	}
	return rtn_val;
}

U8 IOCBI_GetControlStatus(IOCBI_IOHandleType ioHandle)
{
	return iocbiControlStatus[ioHandle];
}

void IOCBI_ReturnAllControlToECU(void)
{
	U16 ioHandle = 0x00u;
	for (ioHandle = 0x00u; ioHandle < IOCBI_IO_MAX; ioHandle++)
	{
		iocbiControlStatus[ioHandle] = IOCBI_RETURNCONTROLTOECU;
	}
}

// Non bit mapped handle saves U8 control value
static U16 IOCBI_NonBitMappedHandle(U8 didIndex, U8 control_type, const volatile U8 *state_record)
{
	U16 rtn_val = S_DSM_RC_OK;
	U16 ioIndex = 0x00u;
	IOCBI_IOCfgType *prtIOCfg;
	IOCBI_DIDCfgType *prtDIDCfg;
	U8 state_record_len = 0;

	U32 TimeValueGet = appclock_gettick_1ms();

	prtDIDCfg = (IOCBI_DIDCfgType *)&IOCBI_DIDCfg[didIndex];
	state_record_len = prtDIDCfg->State_Mask_len;

	for (ioIndex = 0x00u; ioIndex < IOCBI_IO_MAX; ioIndex++)
	{
		prtIOCfg = (IOCBI_IOCfgType *)&IOCBI_IOCfg[ioIndex];
		if (didIndex == prtIOCfg->DIDIndex)
		{
			if (IOCBI_SHORTTERMADJUSTMENT == control_type)
			{
				Std_DataCopy(state_record, prtDIDCfg->dataBuf, state_record_len);
				iocbiControlStatus[ioIndex] = IOCBI_SHORTTERMADJUSTMENT;

				s_SOHControlValue[prtIOCfg->SOHhandle] = *state_record;
				s_SOHControlStatus[prtIOCfg->SOHhandle] = IOCBI_SHORTTERMADJUSTMENT;

				if ((prtIOCfg->safeTime != IOCBI_NO_SAFE_TIME) && (prtIOCfg->prtSafeTimeCnt != NULL))
				{
					(*prtIOCfg->prtSafeTimeCnt) = TimeValueGet;
				}
			}
			else if (IOCBI_RETURNCONTROLTOECU == control_type)
			{
				iocbiControlStatus[ioIndex] = IOCBI_RETURNCONTROLTOECU;
				s_SOHControlStatus[prtIOCfg->SOHhandle] = IOCBI_RETURNCONTROLTOECU;
			}
#if (IOCBI_SUPPORT_FREEZE == ON)
			else if (IOCBI_FREEZECONTROLSTATUS == control_type)
			{
				iocbiControlStatus[ioIndex] = IOCBI_FREEZECONTROLSTATUS;
			}
#endif
			else
			{
				rtn_val = S_DSM_RC_SFNS;
			}
		}
	}
	return rtn_val;
}

// Bit mapped Hanle saves boolean control value
static U16 IOCBI_BitMappedHandle(U8 didIndex, U8 control_type, const volatile U8 *state_record, const volatile U32 mask)
{
	U16 rtn_val = S_DSM_RC_OK;
	U16 ioIndex = 0x00u;
	U8 byteOffset = 0x00u;
	U8 bitOffset = 0x00u;
	U8 bitMask = 0x00u;
	IOCBI_IOCfgType *prtIOCfg;
	IOCBI_DIDCfgType *prtDIDCfg;
	U8 mask_record[4] = {0};

	U32 masktemp = mask;
	masktemp = (masktemp << (8 * (4 - IOCBI_DIDCfg[didIndex].State_Mask_len)));
	mask_record[0] = (U8)(masktemp >> 24);
	mask_record[1] = (U8)(masktemp >> 16);
	mask_record[2] = (U8)(masktemp >> 8);
	mask_record[3] = (U8)(masktemp >> 0);

	U32 TimeValueGet = appclock_gettick_1ms();

	prtDIDCfg = (IOCBI_DIDCfgType *)&IOCBI_DIDCfg[didIndex];
	for (ioIndex = 0x00u; ioIndex < IOCBI_IO_MAX; ioIndex++)
	{
		prtIOCfg = (IOCBI_IOCfgType *)&IOCBI_IOCfg[ioIndex];
		if (didIndex == prtIOCfg->DIDIndex)
		{

			byteOffset = prtIOCfg->postion.startBit / 8;
			bitOffset = prtIOCfg->postion.startBit % 8;
			bitMask = (0x01u << bitOffset);
			if (IOCBI_SHORTTERMADJUSTMENT == control_type)
			{
				if (0x00u != (mask_record[byteOffset] & bitMask))
				{
					iocbiControlStatus[ioIndex] = IOCBI_SHORTTERMADJUSTMENT;
					s_SOHControlStatus[prtIOCfg->SOHhandle] = IOCBI_SHORTTERMADJUSTMENT;
					if (0x00 != (state_record[byteOffset] & bitMask))
					{
						prtDIDCfg->dataBuf[byteOffset] |= bitMask;
						s_SOHControlValue[prtIOCfg->SOHhandle] = 1;
					}
					else
					{
						prtDIDCfg->dataBuf[byteOffset] &= (~bitMask);
						s_SOHControlValue[prtIOCfg->SOHhandle] = 0;
					}

					if ((prtIOCfg->safeTime != IOCBI_NO_SAFE_TIME) && (prtIOCfg->prtSafeTimeCnt != NULL))
					{
						(*prtIOCfg->prtSafeTimeCnt) = TimeValueGet;
					}
				}
			}
			else if (IOCBI_RETURNCONTROLTOECU == control_type)
			{
				iocbiControlStatus[ioIndex] = IOCBI_RETURNCONTROLTOECU;
				s_SOHControlStatus[prtIOCfg->SOHhandle] = IOCBI_RETURNCONTROLTOECU;
			}
#if (IOCBI_SUPPORT_FREEZE == ON)
			else if (IOCBI_FREEZECONTROLSTATUS == control_type)
			{
				iocbiControlStatus[ioIndex] = IOCBI_FREEZECONTROLSTATUS;
				s_SOHControlStatus[prtIOCfg->SOHhandle] = IOCBI_FREEZECONTROLSTATUS;
			}
#endif
			else
			{
				rtn_val = S_DSM_RC_SFNS;
			}
		}
	}

	return rtn_val;
}

static void Std_DataCopy(const volatile U8 *src, U8 *tgt, U16 dataLen)
{
	U8 index;

	for (index = 0; index < dataLen; index++)
	{
		tgt[index] = src[index];
	}
}

// ChannelType : IOCBI_CHANNEL_DIO - 0 ; IOCBI_CHANNEL_PWM - 1
void IOCBI_WriteChannel (U16 channelID, U16 Level, U16 ChannelType)
{
	IOCBI_SignalType* ptrSignal;
	ptrSignal = (IOCBI_SignalType*)&IOCBI_SignalToSOH[channelID];

	if(channelID < SOH_ALL_CHANNEL_MAX)
	{
		if (*ptrSignal->ControlStatus != IOCBI_SHORTTERMADJUSTMENT)
		{
			// DO control
			if (ChannelType == IOCBI_CHANNEL_DIO)
			{
				SOH_WriteChannel((SOH_ALL_ChannelIdType)channelID, Level);
			}
			// PWM control
			else if (ChannelType == IOCBI_CHANNEL_PWM)
			{
				SOH_WriteChannel((SOH_ALL_ChannelIdType)channelID, Level * 10);
			}
		}
		else // under control of iocbi
		{
			// DO control
			if (ChannelType == IOCBI_CHANNEL_DIO)
			{
				if (*ptrSignal->ControlValue == 1)
				{
					SOH_WriteChannel((SOH_ALL_ChannelIdType)channelID, 1);
				}
				else
				{
					SOH_WriteChannel((SOH_ALL_ChannelIdType)channelID, 0);
				}
			}
			// PWM control
			else if (ChannelType == IOCBI_CHANNEL_PWM)
			{
				SOH_WriteChannel((SOH_ALL_ChannelIdType)channelID, (*ptrSignal->ControlValue) * 10);
			}
		}
	}
}
