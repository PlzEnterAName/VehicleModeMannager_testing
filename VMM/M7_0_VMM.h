 /************************Copyright by ofilm 2020-2030*****************************
  * @file    M7_0_VMM.h 
  * @author  Application Team
  * @version V1.0.0
  * @date    26-5-2023
  * @brief   vehicle mode manager
  *******************************************************************************/
#ifndef __M7_0_VMM_H_
#define __M7_0_VMM_H_

#include "M7_0_VMMPublic.h"

typedef enum
{
    MODE_TYPE_FACTORY = 0x00,
    MODE_TYPE_TRANSPORT = 0x01,
    MODE_TYPE_USER = 0x02,
    MODE_TYPE_DYNO = 0x03,
    MODE_TYPE_CRASH = 0x04,
    MODE_TYPE_FACTORY_PAUSED = 0x05,
    MODE_TYPE_TRANSPORT_PAUSED = 0x06,
    MODE_TYPE_MAX_CNT,
    MODE_TYPE_INIT = 0xFF
}M7_0_VEHICLE_MODE_TYPE;

typedef enum
{
    TEL_DIAG_NONE = 0x00,
    TEL_DIAG_FACTORY_PAUSED_MODE = 0x01,
    TEL_DIAG_TRANSPORT_MODE = 0x02,
    TEL_DIAG_USER_MODE = 0x03,
    TEL_DIAG_DYNO_MODE = 0x04,
    TEL_DIAG_MAX
}M7_0_VEHICLE_DIAG_TEL;

typedef enum
{
    ROUTINE_START = 0x01,
    ROUTINE_COMPLETED = 0x02,
    ROUTINE_IN_PROGRESS = 0x03,
    ROUTINE_STOPPED = 0x04,
    ROUTINE_FAILURE = 0x05
}M7_0_ROUTINE_STATUS;

typedef union
{
    uint8 value;
    struct
    {
        uint8 PwrModMisMatch  :1;
        uint8 SpeedMisMatch   :1;
        uint8 KeyInvalid      :1;
        uint8 CurModeWrong    :1;
        uint8 Reserved        :3;
        uint8 RequestModeErr  :1;
    }bit;
}M7_0_ROUTINE_FAIL_REASON;

typedef struct 
{
    uint8 DiagStatus;
    uint8 FailReason;
}VMM_RoutineInquiryData;

typedef enum
{
    WAIT_FOR_POWER_OFF = 0,
    WAIT_FOR_POWER_ON
}M7_0_POWER_ON_OFF_COUNT;

typedef enum
{
    FIND_KEY_START = 0,
    FIND_KEY_WAIT_FOR_SUCCESS,
    FIND_KEY_SUCCESS,
    FIND_KEY_TIME_OUT,
}M7_0_FIND_KEY_STATE;

typedef enum 
{
    SUB_STATE_NONE = 0,
    SUB_STATE_STEP1,
    SUB_STATE_STEP2,
    SUB_STATE_STEP3,
    SUB_STATE_STEP4,
    SUB_STATE_STEP5,
    SUB_STATE_STEP6,
    SUB_STATE_STEP7,
    SUB_STATE_STEP8,
    SUB_STATE_STEP9,
    SUB_STATE_STEP10,
}M7_0_SUB_STATE;

typedef enum
{
    VMM_EVENT_NONE = 0,
    VMM_EVENT_FACTORY_TO_USER,
    VMM_EVENT_FACTORY_TO_FACTORY_PAUSED,
    VMM_EVENT_FACTORY_PAUSED_TO_FACTORY,
    VMM_EVENT_FACTORY_PAUSED_TO_TRANSPORT,
    VMM_EVENT_FACTORY_PAUSED_TO_USER,
    VMM_EVENT_TRANSPORT_TO_USER,
    VMM_EVENT_TRANSPORT_TO_TRANPORT_PAUSED,
    VMM_EVENT_TRANSPORT_PAUSED_TO_TRANPORT,
    VMM_EVENT_TRANSPORT_TO_FACTORY_PAUSED,
    VMM_EVENT_DYNO_TO_USER,
    VMM_EVENT_CRASH_TO_USER,
    VMM_EVENT_CRASH_TO_FACTORY_PAUSED,
    VMM_EVENT_CRASH_TO_TRANSPORT,
    VMM_EVENT_USER_TO_DYNO,
    VMM_EVENT_USER_TO_FACTORY_PAUSED,
    VMM_EVENT_USER_TO_TRANSPORT,
    VMM_EVENT_CRASH, /* all modes can switch to crash mode */
}M7_0_VMM_EVENT;

/* Threshould Value */
#define SPEED_THRESHOULD_STILL 4.0f /* speed below 4km/h means still */
#define POWER_ON_OFF_CNT_50 50
#define POWER_ON_OFF_CNT_100 100

/* Time Count */
#define TIME_LMT_3MIN   (3 * 60 * 1000)
#define TIME_LMT_3SEC   (3 * 1000)
#define TIME_LMT_5SEC   (5 * 1000)
#define TIME_LMT_10SEC  (10 * 1000)

#define SPEED_VALID 0
#define SPEED_INVALID 1

#define SPEED_FACTOR 0.0625


void M7_0_Vehicle_Mode_Init(void);
void M7_0_VehicleModeManagerRunnable(void);

void M7_0_VMM_DiagRequestStart(uint8 mode);
void M7_0_VMM_DiagRequestStop(void);
void M7_0_VMM_DiagInquiry(VMM_RoutineInquiryData *DiagReturnValue);


#endif
